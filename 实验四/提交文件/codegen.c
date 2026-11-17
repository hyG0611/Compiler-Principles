#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "lrparser.tab.h"

typedef struct {
    char operand[32];
    int boolean;
    int is_void;
    int is_float;
    const char *global_name;
    past constant_subscript;
    past dimensions;
    int alignment;
    int array_decay;
} Value;

typedef struct Binding {
    const char *name;
    Value address;
    int has_constant;
    int constant;
    int has_float_constant;
    float float_constant;
    past dimensions;
    past declaration;
    struct Binding *next;
} Binding;

static Binding *locals;
static int temporary, label, current_block, terminated;
static int break_target, continue_target;
static int sample_output, next_loop_metadata;
static Value last_value, return_slot;
static int return_target, entry_number, returns_void, returns_float;
static past functions;
static const char *current_function;
static char *module_globals;
static void emit_global_definition(past global);

typedef struct ArrayConstant {
    char *name;
    struct ArrayConstant *next;
} ArrayConstant;

static ArrayConstant *array_constants;

typedef struct GlobalArrayLayout {
    past declaration;
    char *storage_type;
    struct GlobalArrayLayout *next;
} GlobalArrayLayout;

static GlobalArrayLayout *global_array_layouts;

typedef struct FunctionDeclaration {
    const char *name;
    const char *return_type;
    const char *parameters;
    int emitted;
    struct FunctionDeclaration *next;
} FunctionDeclaration;

static FunctionDeclaration *declarations;
static FunctionDeclaration **declaration_end = &declarations;

static void fail(const char *message)
{
    fprintf(stderr, "Code generation: %s\n", message);
    exit(EXIT_FAILURE);
}

static void emit(char *result, const char *format, ...)
{
    size_t used = strlen(result);
    if (used >= IR_CAPACITY) fail("IR buffer exhausted");
    va_list args;
    va_start(args, format);
    int count = vsnprintf(result + used, IR_CAPACITY - used, format, args);
    va_end(args);
    if (count < 0 || (size_t)count >= IR_CAPACITY - used)
        fail("IR buffer exhausted");
}

/* Record required declarations once, in the order the calls are generated. */
static void require_declaration(const char *name, const char *return_type,
                                const char *parameters)
{
    if (sample_output) return;
    for (FunctionDeclaration *item = declarations; item; item = item->next)
        if (strcmp(item->name, name) == 0) return;

    FunctionDeclaration *item = (FunctionDeclaration *)malloc(sizeof *item);
    if (!item) fail("out of memory");
    item->name = name;
    item->return_type = return_type;
    item->parameters = parameters;
    item->emitted = 0;
    item->next = NULL;
    *declaration_end = item;
    declaration_end = &item->next;
}

static void emit_declarations(char *result)
{
    for (FunctionDeclaration *item = declarations; item; item = item->next) {
        if (item->emitted) continue;
        emit(result, "declare %s @%s(%s)\n", item->return_type, item->name, item->parameters);
        item->emitted = 1;
    }
}

static void free_declarations(void)
{
    while (declarations) {
        FunctionDeclaration *item = declarations;
        declarations = item->next;
        free(item);
    }
    declaration_end = &declarations;
}

static Value fresh(int boolean)
{
    Value value = {.boolean = boolean};
    snprintf(value.operand, sizeof value.operand, "%%%d", ++temporary);
    return value;
}

static Binding *find_local(const char *name)
{
    for (Binding *binding = locals; binding; binding = binding->next)
        if (strcmp(binding->name, name) == 0)
            return binding;
    return NULL;
}

static Binding *push_binding(past declaration)
{
    Binding *binding = (Binding *)calloc(1, sizeof *binding);
    if (!binding) fail("out of memory");
    binding->name = declaration->svalue;
    binding->declaration = declaration;
    binding->dimensions = declaration->right;
    binding->address.is_float = declaration->is_float;
    binding->next = locals;
    locals = binding;
    return binding;
}

static void pop_bindings(Binding *outer)
{
    while (locals != outer) {
        Binding *binding = locals;
        locals = binding->next;
        free(binding);
    }
}

static past find_global(const char *name)
{
    for (past node = functions; node; node = node->next)
        if (node->nodeType == VAR_DECL && strcmp(node->svalue, name) == 0)
            return node;
    return NULL;
}

static Value variable_address(past node)
{
    if (!node || node->nodeType != DECL_REF_EXPR)
        fail("expected a scalar variable");
    Binding *binding = find_local(node->svalue);
    if (binding) return binding->address;
    past global = find_global(node->svalue);
    if (global) {
        emit_global_definition(global);
        return (Value){.is_float = global->is_float, .global_name = global->svalue};
    }
    fail(node->svalue);
    return (Value){0};
}

static void start_block(int block, char *result)
{
    emit(result, "%sbb%d:\n", sample_output ? "\n" : "", block);
    current_block = block;
    terminated = 0;
}

static void jump(int target, char *result)
{
    emit(result, "  br label %%bb%d\n", target);
    terminated = 1;
}

static void branch(Value condition, int yes, int no, char *result)
{
    emit(result, "  br i1 %s, label %%bb%d, label %%bb%d\n",
         condition.operand, yes, no);
    terminated = 1;
}


static const char *value_type(Value value)
{
    return value.is_float ? "float" : "i32";
}

static Value float_value(float number)
{
    Value value = {.is_float = 1};
    snprintf(value.operand, sizeof value.operand, "%.6e", (double)number);
    if (!isfinite(number) || strtod(value.operand, NULL) != (double)number) {
        double wide = number;
        uint64_t bits;
        memcpy(&bits, &wide, sizeof bits);
        snprintf(value.operand, sizeof value.operand, "0x%016llX", (unsigned long long)bits);
    }
    return value;
}

static double literal_float(Value value)
{
    if (value.operand[0] == '0' && value.operand[1] == 'x') {
        uint64_t bits = strtoull(value.operand + 2, NULL, 16);
        double number;
        memcpy(&number, &bits, sizeof number);
        return number;
    }
    return strtod(value.operand, NULL);
}

static Value as_int(Value value, char *result)
{
    if (value.is_void) fail("void function has no value");

    if (value.is_float) {
        Value converted = fresh(0);
        if (value.operand[0] != '%') {
            double number = literal_float(value);
            if (!(number >= INT_MIN && number <= INT_MAX)) fail("float-to-int conversion out of range");
            --temporary;
            snprintf(converted.operand, sizeof converted.operand, "%d", (int)number);
        } else {
            emit(result, "  %s = fptosi float %s to i32\n", converted.operand, value.operand);
        }
        return converted;
    }
    if (!value.boolean) return value;
    if (value.operand[0] != '%') {
        strcpy(value.operand, value.operand[0] == 't' ? "1" : "0");
        value.boolean = 0;
        return value;
    }
    Value converted = fresh(0);
    emit(result, "  %s = zext i1 %s to i32\n", converted.operand, value.operand);
    return converted;
}


static Value as_float(Value value, char *result)
{
    if (value.is_float) return value;
    value = as_int(value, result);
    if (value.operand[0] != '%') return float_value((float)atoi(value.operand));
    Value converted = fresh(0);
    converted.is_float = 1;
    emit(result, "  %s = sitofp i32 %s to float\n", converted.operand, value.operand);
    return converted;
}

static Value convert_value(Value value, int to_float, char *result)
{
    return to_float ? as_float(value, result) : as_int(value, result);
}

static Value as_bool(Value value, char *result)
{
    if (value.is_void) fail("void function has no value");
    if (value.boolean) return value;
    if (value.operand[0] != '%') {
        int truth = value.is_float ? literal_float(value) != 0 : atoi(value.operand) != 0;
        strcpy(value.operand, truth ? "true" : "false");
        value.boolean = 1;
        value.is_float = 0;
        return value;
    }
    Value converted = fresh(1);

    if (value.is_float)
        emit(result, "  %s = fcmp une float %s, 0.000000e+00\n", converted.operand, value.operand);
    else
        emit(result, "  %s = icmp ne i32 %s, 0\n", converted.operand, value.operand);

    return converted;
}

static const char *arithmetic(int op)
{
    switch (op) {
    case Y_ADD: return "add";
    case Y_SUB: return "sub";
    case Y_MUL: return "mul";
    case Y_DIV: return "sdiv";
    case Y_MODULO: return "srem";
    default: return NULL;
    }
}

static const char *predicate(int op)
{
    switch (op) {
    case Y_LESS: return "slt";
    case Y_LESSEQ: return "sle";
    case Y_GREAT: return "sgt";
    case Y_GREATEQ: return "sge";
    case Y_EQ: return "eq";
    case Y_NOTEQ: return "ne";
    default: fail("unsupported comparison"); return NULL;
    }
}

static int constant_float(past node, float *value);
static int constant_truth(past node, int *truth);

static int constant_int(past node, int *result)
{
    int left, right;
    int64_t value;
    switch (node->nodeType) {
    case INTEGER_LITERAL:
        *result = node->ivalue;
        return 1;
    case PAREN_EXPR:
        return constant_int(node->left, result);
    case UNARY_OPERATOR:
        if (node->ivalue == Y_NOT) {
            if (!constant_truth(node->left, &left)) return 0;
            *result = !left;
            return 1;
        }
        if (!constant_int(node->left, &left)) return 0;
        switch (node->ivalue) {
        case Y_ADD: value = left; break;
        case Y_SUB: value = -(int64_t)left; break;
        default: return 0;
        }
        break;
    case BINARY_OPERATOR:
        if ((node->ivalue == Y_LESS || node->ivalue == Y_LESSEQ ||
             node->ivalue == Y_GREAT || node->ivalue == Y_GREATEQ ||
             node->ivalue == Y_EQ || node->ivalue == Y_NOTEQ) &&
            (!constant_int(node->left, &left) || !constant_int(node->right, &right))) {
            float lhs, rhs;
            if (!constant_float(node->left, &lhs) || !constant_float(node->right, &rhs)) return 0;
            switch (node->ivalue) {
            case Y_LESS: *result = lhs < rhs; return 1;
            case Y_LESSEQ: *result = lhs <= rhs; return 1;
            case Y_GREAT: *result = lhs > rhs; return 1;
            case Y_GREATEQ: *result = lhs >= rhs; return 1;
            case Y_EQ: *result = lhs == rhs; return 1;
            case Y_NOTEQ: *result = lhs != rhs; return 1;
            }
        }
        if (node->ivalue == Y_AND || node->ivalue == Y_OR) {
            if (!constant_truth(node->left, &left)) return 0;
            if ((node->ivalue == Y_AND && !left) || (node->ivalue == Y_OR && left)) {
                *result = left;
                return 1;
            }
            return constant_truth(node->right, result);
        }
        if (!constant_int(node->left, &left)) return 0;
        if (!constant_int(node->right, &right)) return 0;
        switch (node->ivalue) {
        case Y_ADD: value = (int64_t)left + right; break;
        case Y_SUB: value = (int64_t)left - right; break;
        case Y_MUL: value = (int64_t)left * right; break;
        case Y_LESS: value = left < right; break;
        case Y_LESSEQ: value = left <= right; break;
        case Y_GREAT: value = left > right; break;
        case Y_GREATEQ: value = left >= right; break;
        case Y_EQ: value = left == right; break;
        case Y_NOTEQ: value = left != right; break;
        case Y_DIV:
            if (right == 0 || (left == INT_MIN && right == -1)) return 0;
            value = left / right;
            break;
        case Y_MODULO:
            if (right == 0 || (left == INT_MIN && right == -1)) return 0;
            value = left % right;
            break;
        default: return 0;
        }
        break;
    default:
        return 0;
    }

    if (value < INT_MIN || value > INT_MAX) return 0;
    *result = (int)value;
    return 1;
}

static int constant_truth(past node, int *truth)
{
    while (node->nodeType == PAREN_EXPR) node = node->left;
    int integer;
    if (constant_int(node, &integer)) {
        *truth = integer != 0;
        return 1;
    }
    /* Failed integer truth expressions cannot become float constants. */
    if ((node->nodeType == BINARY_OPERATOR && !arithmetic(node->ivalue)) ||
        (node->nodeType == UNARY_OPERATOR && node->ivalue == Y_NOT)) return 0;
    float floating;
    if (!constant_float(node, &floating)) return 0;
    *truth = floating != 0.0f;
    return 1;
}

/* Convert only at an integer initialization boundary. */
static int constant_int_initializer(past node, int *value)
{
    if (constant_int(node, value)) return 1;
    float literal;
    if (!constant_float(node, &literal)) return 0;
    double number = literal;
    if (!(number >= INT_MIN && number <= INT_MAX)) return 0;
    *value = (int)number;
    return 1;
}

/* Follow the same brace grouping and implicit zero fill as initialize_array. */
static int read_constant_element(past dim, past *item, size_t stride,
                                 size_t *offset, past *value)
{
    int count;
    if (!constant_int(dim->right, &count) || count <= 0) return -1;
    size_t width = stride / (size_t)count;
    for (int i = 0; i < count; ++i) {
        if (!*item) {
            size_t missing = (size_t)(count - i) * width;
            if (*offset < missing) { *value = NULL; return 1; }
            *offset -= missing;
            return 0;
        }
        if (dim->next) {
            int found;
            if ((*item)->nodeType == INIT_LIST_EXPR) {
                past nested = (*item)->left;
                *item = (*item)->next;
                found = read_constant_element(dim->next, &nested, width, offset, value);
            } else {
                found = read_constant_element(dim->next, item, width, offset, value);
            }
            if (found) return found;
        } else {
            past scalar = *item;
            *item = scalar->next;
            if (*offset) { --*offset; continue; }
            while (scalar && scalar->nodeType == INIT_LIST_EXPR) scalar = scalar->left;
            *value = scalar;
            return 1;
        }
    }
    return 0;
}

static int fold_constant_array_element(past subscript)
{
    int rank = 0;
    past base = subscript;
    while (base->nodeType == ARRAY_SUBSCRIPT_EXPR) { ++rank; base = base->left; }
    if (base->nodeType != DECL_REF_EXPR) return 0;
    Binding *binding = find_local(base->svalue);
    if (!binding) return 0;
    past decl = binding->declaration;
    if (!decl->is_const || !decl->left ||
        decl->left->nodeType != INIT_LIST_EXPR) return 0;

    int dimensions = 0;
    for (past dim = decl->right; dim; dim = dim->next) ++dimensions;
    if (dimensions != rank) return 0;
    size_t stride = 1, offset = 0;
    past index_node = subscript;
    for (int axis = rank; axis > 0; --axis, index_node = index_node->left) {
        past dim = decl->right;
        for (int i = 1; i < axis; ++i) dim = dim->next;
        int index, bound;
        if (!constant_int(dim->right, &bound) || bound <= 0 ||
            !constant_int(index_node->right, &index) || index < 0 || index >= bound ||
            stride > SIZE_MAX / (size_t)bound) return 0;
        offset += stride * (size_t)index;
        stride *= (size_t)bound;
    }
    past item = decl->left->left, scalar = NULL;
    if (read_constant_element(decl->right, &item, stride, &offset, &scalar) != 1) return 0;
    if (decl->is_float) {
        float value;
        if (!constant_float(scalar, &value)) return 0;
        subscript->nodeType = FLOATING_LITERAL;
        subscript->fvalue = value;
    } else {
        int value = 0;
        if (scalar && !constant_int_initializer(scalar, &value)) return 0;
        subscript->nodeType = INTEGER_LITERAL;
        subscript->ivalue = value;
    }
    return 1;
}

static void resolve_constants(past node, int required);

/* Resolve references in a writable location's indices, keeping its base. */
static void resolve_indices(past node)
{
    if (node->nodeType != ARRAY_SUBSCRIPT_EXPR) return;
    resolve_indices(node->left);
    resolve_constants(node->right, 0);
}

static void resolve_constants(past node, int required)
{
    for (; node; node = node->next) {
        switch (node->nodeType) {
        case FUNCTION_DECL: {
            Binding *outer = locals;
            for (past param = node->left; param; param = param->next) {
                resolve_constants(param->right, 1);
                push_binding(param);
            }
            resolve_constants(node->right, 0);
            pop_bindings(outer);
            break;
        }
        case COMPOUND_STMT: {
            Binding *outer = locals;
            resolve_constants(node->left, required);
            pop_bindings(outer);
            break;
        }
        case VAR_DECL: {
            /* Scalar initializers may be surrounded by one or more brace pairs. */
            if (!node->right) {
                while (node->left && node->left->nodeType == INIT_LIST_EXPR) {
                    past value = node->left->left;
                    if (value && value->next) fail("too many scalar initializers");
                    node->left = value ? value : newInt(0);
                }
            }
            resolve_constants(node->right, 1);
            Binding *binding = push_binding(node);
            resolve_constants(node->left, required || node->is_const);
            binding->has_constant = node->is_const && !node->is_float && !node->right && node->left &&
                                    constant_int_initializer(node->left, &binding->constant);
            binding->has_float_constant = node->is_const && node->is_float &&
                !node->right && node->left && constant_float(node->left, &binding->float_constant);
            break;
        }
        case DECL_REF_EXPR: {
            Binding *binding = find_local(node->svalue);
            if (binding && binding->has_constant) {
                node->nodeType = INTEGER_LITERAL;
                node->ivalue = binding->constant;
            } else if (binding && binding->has_float_constant) {
                node->nodeType = FLOATING_LITERAL;
                node->fvalue = binding->float_constant;
            }
            break;
        }
        case ARRAY_SUBSCRIPT_EXPR: {
            resolve_constants(node->left, required);
            resolve_constants(node->right, required);
            if (required && node->left) fold_constant_array_element(node);
            break;
        }
        case CALL_EXPR:
            resolve_constants(node->right, required);
            break;
        case BINARY_OPERATOR:
            if (node->ivalue == Y_ASSIGN) {
                resolve_indices(node->left);
                resolve_constants(node->right, required);
                break;
            }
            /* fall through */
        default:
            resolve_constants(node->left, required);
            resolve_constants(node->right, required);
            resolve_constants(node->if_cond, required);
            break;
        }
    }
}

static Value expression(past node, char *result);

enum { TYPE_CAPACITY = 256 };

static int array_bound(past dimension)
{
    int size;
    if (!constant_int(dimension->right, &size) || size < 0)
        fail("array bounds must be nonnegative integer constants");
    return size;
}

static void array_type(past dimension, int is_float, char type[TYPE_CAPACITY])
{
    size_t used = 0, depth = 0;
    for (; dimension; dimension = dimension->next) {
        int written = snprintf(type + used, TYPE_CAPACITY - used,
                               "[%d x ", array_bound(dimension));
        if (written < 0 || (size_t)written >= TYPE_CAPACITY - used)
            fail("array type is too long");
        used += (size_t)written;
        ++depth;
    }
    const char *scalar = is_float ? "float" : "i32";
    size_t length = strlen(scalar);
    if (used + depth + length + 1 > TYPE_CAPACITY) fail("array type is too long");
    memcpy(type + used, scalar, length);
    used += length;
    memset(type + used, ']', depth);
    type[used + depth] = '\0';
}

static int array_alignment(past dimensions)
{
    int elements = 1;
    for (; dimensions; dimensions = dimensions->next) {
        int bound = array_bound(dimensions);
        if (bound == 0) return 4;
        if (elements < 4) elements = bound >= 4 ? 4 : elements * bound;
    }
    return elements >= 4 ? 16 : 4;
}

static void emit_pointer(Value pointer, char *result);
static Value constant_array_address(past node, past *remaining);

static Value array_element(Value base, past dimensions, Value index, char *result)
{
    char type[TYPE_CAPACITY];
    int is_parameter = !dimensions->right;
    array_type(is_parameter ? dimensions->next : dimensions, base.is_float, type);
    index = as_int(index, result);
    int stride = 4;
    for (past dim = dimensions->next; dim; dim = dim->next)
        stride = (stride * (array_bound(dim) % 16)) % 16;
    int offset = stride;
    if (index.operand[0] != '%')
        offset = ((atoi(index.operand) % 16) * stride) % 16;
    int alignment = base.alignment ? base.alignment : 4;
    while (alignment > 4 && offset % alignment) alignment /= 2;
    if (index.operand[0] == '%') {
        Value extended = fresh(0);
        emit(result, "  %s = sext i32 %s to i64\n", extended.operand, index.operand);
        index = extended;
    }
    Value pointer = fresh(0);
    pointer.alignment = alignment;
    pointer.is_float = base.is_float;
    emit(result, "  %s = getelementptr inbounds %s, %s* ", pointer.operand, type, type);
    emit_pointer(base, result);
    emit(result, ", %si64 %s\n", is_parameter ? "" : "i64 0, ", index.operand);
    return pointer;
}

static Value indexed_address(past node, past *dimensions, char *result)
{
    if (node->nodeType == ARRAY_SUBSCRIPT_EXPR) {
        Value constant = constant_array_address(node, dimensions);
        if (constant.constant_subscript) return constant;
        Value base = indexed_address(node->left, dimensions, result);
        if (!*dimensions) fail("too many array subscripts");
        Value index = expression(node->right, result);
        Value element = array_element(base, *dimensions, index, result);
        *dimensions = (*dimensions)->next;
        return element;
    }

    Value base = variable_address(node);
    Binding *local = find_local(node->svalue);
    *dimensions = local ? local->dimensions : find_global(node->svalue)->right;
    if (*dimensions && (*dimensions)->right)
        base.alignment = array_alignment(*dimensions);
    if (*dimensions && !(*dimensions)->right) {
        char type[TYPE_CAPACITY];
        array_type((*dimensions)->next, base.is_float, type);
        Value pointer = fresh(0);
        pointer.is_float = base.is_float;
        emit(result, "  %s = load %s*, %s** %s, align 8\n",
             pointer.operand, type, type, base.operand);
        return pointer;
    }
    return base;
}

/* Keep constant global indices in one address, including partial subscripts. */
static Value constant_array_address(past node, past *remaining)
{
    past base = node;
    int subscripts = 0;
    while (base->nodeType == ARRAY_SUBSCRIPT_EXPR) {
        int index;
        if (!constant_int(base->right, &index)) return (Value){0};
        ++subscripts;
        base = base->left;
    }
    if (base->nodeType != DECL_REF_EXPR || find_local(base->svalue))
        return (Value){0};
    past global = find_global(base->svalue);
    if (!global) return (Value){0};

    past tail = global->right;
    for (int i = 0; i < subscripts; ++i) {
        if (!tail) return (Value){0};
        tail = tail->next;
    }
    *remaining = tail;
    emit_global_definition(global);

    /* Only the offset modulo four elements affects alignment up to 16. */
    int offset = 0, stride = 1;
    for (past dim = tail; dim; dim = dim->next)
        stride = (stride * (array_bound(dim) % 4)) % 4;
    past subscript = node;
    for (int axis = subscripts; axis > 0; --axis) {
        int index = 0;
        constant_int(subscript->right, &index);
        offset = (offset + (index % 4) * stride) % 4;
        past dim = global->right;
        for (int i = 1; i < axis; ++i) dim = dim->next;
        stride = (stride * (array_bound(dim) % 4)) % 4;
        subscript = subscript->left;
    }

    int alignment = array_alignment(global->right);
    if (offset % 2) alignment = 4;
    else if (offset && alignment > 8) alignment = 8;
    return (Value){.is_float = global->is_float, .global_name = global->svalue,
                   .constant_subscript = node, .dimensions = global->right, .alignment = alignment};
}

/* A multidimensional constant address is one GEP with all indices. */
static void emit_constant_indices(past subscript, char *result)
{
    if (subscript->nodeType != ARRAY_SUBSCRIPT_EXPR) return;
    emit_constant_indices(subscript->left, result);
    int index = 0;
    constant_int(subscript->right, &index);
    emit(result, ", i64 %d", index);
}

static void emit_global_address(const char *name, char *result)
{
    for (GlobalArrayLayout *layout = global_array_layouts; layout; layout = layout->next) {
        past decl = layout->declaration;
        if (strcmp(decl->svalue, name) != 0) continue;
        char expected[TYPE_CAPACITY];
        array_type(decl->right, decl->is_float, expected);
        emit(result, "bitcast (%s* @%s to %s*)", layout->storage_type, name, expected);
        return;
    }
    emit(result, "@%s", name);
}

static void emit_pointer(Value pointer, char *result)
{
    if (pointer.constant_subscript) {
        char type[TYPE_CAPACITY];
        array_type(pointer.dimensions, pointer.is_float, type);
        emit(result, "getelementptr inbounds (%s, %s* ", type, type);
        emit_global_address(pointer.global_name, result);
        emit(result, ", i64 0");
        emit_constant_indices(pointer.constant_subscript, result);
        if (pointer.array_decay) emit(result, ", i64 0");
        emit(result, ")");
    } else if (pointer.array_decay) {
        char type[TYPE_CAPACITY];
        array_type(pointer.dimensions, pointer.is_float, type);
        emit(result, "getelementptr inbounds (%s, %s* ", type, type);
        emit_global_address(pointer.global_name, result);
        emit(result, ", i64 0, i64 0)");
    } else if (pointer.global_name) {
        emit_global_address(pointer.global_name, result);
    } else {
        emit(result, "%s", pointer.operand);
    }
}

static Value address(past node, char *result)
{
    past dimensions;
    Value pointer = indexed_address(node, &dimensions, result);
    if (dimensions) fail("expected an integer array element");
    return pointer;
}

static Value array_argument(past node, past element_dimensions, int element_is_float, char *result)
{
    past dimensions;
    while (node->nodeType == PAREN_EXPR) node = node->left;
    /* Decaying a constant-indexed global subarray stays a constant GEP. */
    past root = node;
    int rank = 0, constant = 1;
    while (root->nodeType == ARRAY_SUBSCRIPT_EXPR) {
        int index;
        if (!constant_int(root->right, &index)) constant = 0;
        ++rank;
        root = root->left;
    }
    past global = root->nodeType == DECL_REF_EXPR && !find_local(root->svalue)
        ? find_global(root->svalue) : NULL;
    if (constant && global) {
        past remaining = global->right;
        for (int i = 0; i < rank && remaining; ++i) remaining = remaining->next;
        if (!remaining) fail("expected an array argument");
        char actual[TYPE_CAPACITY], expected[TYPE_CAPACITY];
        array_type(remaining->next, global->is_float, actual);
        array_type(element_dimensions, element_is_float, expected);
        if (strcmp(actual, expected) != 0) fail("array argument type mismatch");
        emit_global_definition(global);
        return (Value){.is_float = global->is_float, .global_name = global->svalue,
                       .constant_subscript = node, .dimensions = global->right, .array_decay = 1};
    }
    Value base = indexed_address(node, &dimensions, result);
    if (!dimensions) fail("expected an array argument");

    char actual[TYPE_CAPACITY], expected[TYPE_CAPACITY];
    array_type(dimensions->next, base.is_float, actual);
    array_type(element_dimensions, element_is_float, expected);
    if (strcmp(actual, expected) != 0) fail("array argument type mismatch");

    if (!dimensions->right) return base;
    if (base.global_name) {
        base.array_decay = 1;
        base.dimensions = dimensions;
        return base;
    }
    return array_element(base, dimensions, (Value){.operand = "0"}, result);
}


static Value runtime_array_call(past node, int is_get, int is_float, char *result)
{
    past first = node->right;
    past array_node = is_get || !first ? first : first->next;
    if (!array_node || array_node->next) fail("wrong number of array library arguments");
    const char *name = node->left->svalue;
    const char *type = is_float ? "float" : "i32";
    const char *parameters = is_get ? (is_float ? "float*" : "i32*")
                                    : (is_float ? "i32, float*" : "i32, i32*");
    require_declaration(name, is_get ? "i32" : "void", parameters);

    Value length = {0};
    if (!is_get) length = as_int(expression(first, result), result);
    Value array = array_argument(array_node, NULL, is_float, result);
    if (is_get) {
        Value count = fresh(0);
        emit(result, "  %s = call i32 @%s(%s*%s ", count.operand, name, type,
             sample_output ? "" : " noundef");
        emit_pointer(array, result);
        emit(result, ")\n");
        return count;
    }

    const char *attribute = sample_output ? "" : " noundef";
    emit(result, "  call void @%s(i32%s %s, %s*%s ", name, attribute, length.operand, type, attribute);
    emit_pointer(array, result);
    emit(result, ")\n");
    return (Value){.is_void = 1};
}

static Value external_call(past node, char *result)
{
    const char *name = node->left->svalue;
    if (strcmp(name, "getarray") == 0) return runtime_array_call(node, 1, 0, result);
    if (strcmp(name, "putarray") == 0) return runtime_array_call(node, 0, 0, result);
    if (strcmp(name, "getfarray") == 0) return runtime_array_call(node, 1, 1, result);
    if (strcmp(name, "putfarray") == 0) return runtime_array_call(node, 0, 1, result);
    if (strcmp(name, "getfloat") == 0) {
        if (node->right) fail("getfloat expects no arguments");
        require_declaration(name, "float", "");
        Value value = fresh(0);
        value.is_float = 1;
        emit(result, "  %s = call float @getfloat()\n", value.operand);
        return value;
    }
    if (strcmp(name, "putfloat") == 0) {
        if (!node->right || node->right->next) fail("putfloat expects one argument");
        require_declaration(name, "void", "float");
        Value value = as_float(expression(node->right, result), result);
        emit(result, "  call void @putfloat(float%s %s)\n", sample_output ? "" : " noundef", value.operand);
        return (Value){.is_void = 1};
    }
    if (sample_output && strcmp(name, "putint") == 0) {
        if (!node->right || node->right->next) fail("putint expects one argument");
        Value argument = as_int(expression(node->right, result), result);
        Value value = fresh(0);
        /* The course reference prints the result type only for a call argument. */
        const char *type = node->right->nodeType == CALL_EXPR ? " i32" : "";
        emit(result, "  %s = call%s @putint(i32 %s)\n", value.operand, type, argument.operand);
        return value;
    }
    require_declaration(name, "i32", "...");
    size_t count = 0;
    for (past arg = node->right; arg; arg = arg->next) ++count;
    Value *arguments = count ? (Value *)malloc(count * sizeof *arguments) : NULL;
    if (count && !arguments) fail("out of memory");
    size_t index = 0;
    for (past arg = node->right; arg; arg = arg->next) {
        Value value = expression(arg, result);
        if (value.is_float && strcmp(name, "putint") != 0 && strcmp(name, "putch") != 0)
            fail("external float arguments require a prototype");
        arguments[index++] = as_int(value, result);
    }

    Value value = fresh(0);
    if (!count) {
        emit(result, "  %s = call i32 (...) @%s()\n", value.operand, name);
    } else {
        emit(result, "  %s = call i32 (", value.operand);
        for (index = 0; index < count; ++index) emit(result, "i32, ");
        emit(result, "...) bitcast (i32 (...)* @%s to i32 (", name);
        for (index = 0; index < count; ++index) emit(result, "i32, ");
        emit(result, "...)*)(");
        for (index = 0; index < count; ++index)
            emit(result, "%si32%s %s", index ? ", " : "", sample_output ? "" : " noundef",
                 arguments[index].operand);
        emit(result, ")\n");
    }
    free(arguments);
    return value;
}

static Value call(past node, char *result)
{
    const char *name = node->left->svalue;
    past function = functions;
    while (function && (function->nodeType != FUNCTION_DECL ||
                        strcmp(function->svalue, name) != 0))
        function = function->next;
    if (!function) return external_call(node, result);

    size_t count = 0;
    for (past arg = node->right; arg; arg = arg->next) ++count;
    size_t parameters = 0;
    for (past param = function->left; param; param = param->next) ++parameters;
    if (count != parameters) fail("wrong number of arguments");

    Value *arguments = count ? (Value *)malloc(count * sizeof *arguments) : NULL;
    if (count && !arguments) fail("out of memory");
    size_t index = 0;
    past param = function->left;
    for (past arg = node->right; arg; arg = arg->next, param = param->next) {
        arguments[index++] = param->right ? array_argument(arg, param->right->next, param->is_float, result)
                                          : convert_value(expression(arg, result), param->is_float, result);
    }

    Value value = {.is_void = function->ivalue == Y_VOID};
    if (value.is_void) {
        emit(result, "  call void @%s(", name);
    } else {
        value = fresh(0);
        value.is_float = function->is_float;
        emit(result, "  %s = call %s @%s(", value.operand, value_type(value), name);
    }
    param = function->left;
    for (index = 0; index < count; ++index, param = param->next) {
        char type[TYPE_CAPACITY];
        array_type(param->right ? param->right->next : NULL, param->is_float, type);
        emit(result, "%s%s%s%s ", index ? ", " : "", type,
             param->right ? "*" : "", sample_output ? "" : " noundef");
        if (param->right) emit_pointer(arguments[index], result);
        else emit(result, "%s", arguments[index].operand);
    }
    emit(result, ")\n");
    free(arguments);
    return value;
}

static Value expression(past node, char *result)
{
    if (!node) fail("missing expression");
    switch (node->nodeType) {
    case BINARY_OPERATOR:
        if (arithmetic(node->ivalue)) genArithmeticExpr(node, result);
        else genLogicExpr(node, result);
        return last_value;
    case UNARY_OPERATOR:
        if (node->ivalue == Y_NOT) genLogicExpr(node, result);
        else genArithmeticExpr(node, result);
        return last_value;
    default:
        genArithmeticExpr(node, result);
        return last_value;
    }
}


static int constant_float(past node, float *value);

static int expression_is_float(past node)
{
    switch (node->nodeType) {
    case FLOATING_LITERAL: return 1;
    case ARRAY_SUBSCRIPT_EXPR:
    case PAREN_EXPR: return expression_is_float(node->left);
    case UNARY_OPERATOR:
        return node->ivalue != Y_NOT && expression_is_float(node->left);
    case BINARY_OPERATOR:
        return arithmetic(node->ivalue) &&
               (expression_is_float(node->left) || expression_is_float(node->right));
    case DECL_REF_EXPR: {
        Binding *local = find_local(node->svalue);
        past global = local ? NULL : find_global(node->svalue);
        return local ? local->address.is_float : global && global->is_float;
    }
    case CALL_EXPR:
        for (past function = functions; function; function = function->next)
            if (function->nodeType == FUNCTION_DECL &&
                strcmp(function->svalue, node->left->svalue) == 0)
                return function->is_float;
        return strcmp(node->left->svalue, "getfloat") == 0;
    default: return 0;
    }
}

static int arithmetic_node(past node)
{
    return node->nodeType == BINARY_OPERATOR && arithmetic(node->ivalue) != NULL;
}

static int fold_arithmetic(past node, Value *value)
{
    float floating;
    int integer;
    if (expression_is_float(node) && constant_float(node, &floating)) {
        *value = float_value(floating);
        return 1;
    }
    if (!constant_int(node, &integer)) return 0;
    *value = (Value){0};
    snprintf(value->operand, sizeof value->operand, "%d", integer);
    return 1;
}

static Value emit_arithmetic_step(past node, Value left, int floating, char *result)
{
    int op = node->ivalue;
    const char *instruction = arithmetic(op);
    if (!instruction) fail("unsupported arithmetic operator");
    left = convert_value(left, floating, result);
    Value right = convert_value(expression(node->right, result), floating, result);
    Value value = fresh(0);
    value.is_float = floating;
    if (floating) {
        if (op == Y_MODULO) fail("remainder requires integer operands");
        const char *operation = op == Y_ADD ? "fadd" : op == Y_SUB ? "fsub" :
                                op == Y_MUL ? "fmul" : "fdiv";
        emit(result, "  %s = %s float %s, %s\n",
             value.operand, operation, left.operand, right.operand);
    } else {
        const char *flags = op == Y_DIV || op == Y_MODULO ? "" : " nsw";
        emit(result, "  %s = %s%s i32 %s, %s\n", value.operand, instruction,
             flags, left.operand, right.operand);
    }
    return value;
}

/* Bison builds a left-deep AST for arithmetic chains. Unwind long chains
   on the heap, preserving source evaluation order and constant prefixes. */
static Value emit_long_arithmetic(past node, char *result)
{
    size_t count = 0;
    past first = node;
    for (; arithmetic_node(first); first = first->left) ++count;
    past *steps = (past *)malloc(count * sizeof *steps);
    if (!steps) fail("out of memory");
    past current = node;
    for (size_t i = count; i > 0; --i, current = current->left)
        steps[i - 1] = current;

    Value value = {0};
    size_t start = 0;
    while (start < count && fold_arithmetic(steps[start], &value)) ++start;
    if (start == 0) value = expression(first, result);
    for (size_t i = start; i < count; ++i) {
        int floating = value.is_float || expression_is_float(steps[i]->right);
        value = emit_arithmetic_step(steps[i], value, floating, result);
    }
    free(steps);
    return value;
}

int genArithmeticExpr(past node, char *result)
{
    Value value = {0};
    past scan = node;
    size_t depth = 0;
    while (depth < 64 && arithmetic_node(scan)) {
        scan = scan->left;
        ++depth;
    }
    if (depth == 64) {
        last_value = emit_long_arithmetic(node, result);
        return last_value.operand[0] == '%' ? atoi(last_value.operand + 1) : 0;
    }
    if (fold_arithmetic(node, &value)) {
        last_value = value;
        return 0;
    }
    switch (node->nodeType) {
    case FLOATING_LITERAL:
        value = float_value(node->fvalue);
        break;
    case CALL_EXPR:
        value = call(node, result);
        break;
    case ARRAY_SUBSCRIPT_EXPR:
    case DECL_REF_EXPR: {
        Binding *binding = node->nodeType == DECL_REF_EXPR ? find_local(node->svalue) : NULL;
        past global = binding || node->nodeType != DECL_REF_EXPR ? NULL : find_global(node->svalue);
        int immediate;
        if ((binding && binding->has_constant) ||
            (global && global->is_const && !global->is_float && !global->right && global->left && constant_int_initializer(global->left, &immediate))) {
            if (binding) immediate = binding->constant;
            snprintf(value.operand, sizeof value.operand, "%d", immediate);
            break;
        }
        Value ptr = address(node, result);
        value = fresh(0);
        value.is_float = ptr.is_float;
        emit(result, "  %s = load %s, %s* ", value.operand, value_type(value), value_type(value));
        emit_pointer(ptr, result);
        emit(result, ", align %d\n", ptr.alignment ? ptr.alignment : 4);
        break;
    }
    case PAREN_EXPR:
        value = expression(node->left, result);
        break;
    case UNARY_OPERATOR: {
        Value operand = expression(node->left, result);
        if (!operand.is_float) operand = as_int(operand, result);
        value = operand;
        if (node->ivalue == Y_SUB) {
            value = fresh(0);

            value.is_float = operand.is_float;
            if (operand.is_float)
                emit(result, "  %s = fneg float %s\n", value.operand, operand.operand);
            else
                emit(result, "  %s = sub nsw i32 0, %s\n", value.operand, operand.operand);

        } else if (node->ivalue != Y_ADD) fail("unsupported unary operator");
        break;
    }
    case BINARY_OPERATOR: {
        int floating = expression_is_float(node->left) || expression_is_float(node->right);
        Value left = expression(node->left, result);
        value = emit_arithmetic_step(node, left, floating, result);
        break;
    }
    default:
        fail("expected an integer scalar expression");
    }
    last_value = value;
    return value.operand[0] == '%' ? atoi(value.operand + 1) : 0;
}

typedef struct PhiPredecessor {
    int block;
    struct PhiPredecessor *next;
} PhiPredecessor;

static void branch_on_collect(past node, int yes, int no, int merge,
                              PhiPredecessor **predecessors, char *result);

int genLogicExpr(past node, char *result)
{
    int immediate;
    if (constant_truth(node, &immediate)) {
        last_value = (Value){.boolean = 1};
        strcpy(last_value.operand, immediate ? "true" : "false");
        return 0;
    }
    int op = node->ivalue;
    if (node->nodeType == UNARY_OPERATOR && op == Y_NOT) {
        Value operand = as_bool(expression(node->left, result), result);
        last_value = fresh(1);
        emit(result, "  %s = xor i1 %s, true\n", last_value.operand, operand.operand);
    } else if (node->nodeType != BINARY_OPERATOR || arithmetic(op)) {
        last_value = as_bool(expression(node, result), result);
    } else if (op == Y_AND || op == Y_OR) {
        if (constant_truth(node->left, &immediate)) {
            last_value = as_bool(expression(node->right, result), result);
            return 0;
        }
        int rhs = ++label;
        int merge = ++label;
        PhiPredecessor *predecessors = NULL;
        if (op == Y_AND)
            branch_on_collect(node->left, rhs, merge, merge, &predecessors, result);
        else
            branch_on_collect(node->left, merge, rhs, merge, &predecessors, result);

        start_block(rhs, result);
        Value right = as_bool(expression(node->right, result), result);
        int rhs_exit = current_block;
        jump(merge, result);
        start_block(merge, result);
        last_value = fresh(1);
        emit(result, "  %s = phi i1 ", last_value.operand);
        while (predecessors) {
            PhiPredecessor *input = predecessors;
            if (input->block == 0)
                emit(result, "[ %s, %%%d ], ", op == Y_AND ? "false" : "true", entry_number);
            else
                emit(result, "[ %s, %%bb%d ], ", op == Y_AND ? "false" : "true", input->block);
            predecessors = input->next;
            free(input);
        }
        emit(result, "[ %s, %%bb%d ]\n", right.operand, rhs_exit);
    } else {
        const char *comparison = predicate(op);

        int floating = expression_is_float(node->left) || expression_is_float(node->right);
        Value left = convert_value(expression(node->left, result), floating, result);
        Value right = convert_value(expression(node->right, result), floating, result);
        last_value = fresh(1);

        if (floating) {
            const char *test = op == Y_LESS ? "olt" : op == Y_LESSEQ ? "ole" :
                               op == Y_GREAT ? "ogt" : op == Y_GREATEQ ? "oge" :
                               op == Y_EQ ? "oeq" : "une";
            emit(result, "  %s = fcmp %s float %s, %s\n",
                 last_value.operand, test, left.operand, right.operand);
        } else {
            emit(result, "  %s = icmp %s i32 %s, %s\n",
                 last_value.operand, comparison, left.operand, right.operand);
        }

    }
    return 0;
}

static void store(Value value, Value ptr, char *result)
{
    value = convert_value(value, ptr.is_float, result);
    emit(result, "  store %s %s, %s* ", value_type(value), value.operand, value_type(value));
    emit_pointer(ptr, result);
    emit(result, ", align %d\n", ptr.alignment ? ptr.alignment : 4);
}

int genAssignStmt(past node, char *result)
{
    Value value = expression(node->right, result);
    Value ptr = address(node->left, result);
    store(value, ptr, result);
    return 0;
}

static void statement(past node, char *result);

/* In a condition the branch itself carries the logical result. */
static void branch_on_collect(past node, int yes, int no, int merge,
                              PhiPredecessor **predecessors, char *result)
{
    /* Clang folds neutral constant operands before creating short-circuit blocks. */
    int immediate;
    if (node->nodeType == BINARY_OPERATOR &&
        (node->ivalue == Y_AND || node->ivalue == Y_OR)) {
        int is_and = node->ivalue == Y_AND;
        if (constant_truth(node->left, &immediate) && (immediate == is_and)) {
            branch_on_collect(node->right, yes, no, merge, predecessors, result);
            return;
        }
        if (constant_truth(node->right, &immediate) && (immediate == is_and)) {
            branch_on_collect(node->left, yes, no, merge, predecessors, result);
            return;
        }
    }
    if (node->nodeType == PAREN_EXPR) {
        branch_on_collect(node->left, yes, no, merge, predecessors, result);
    } else if (node->nodeType == UNARY_OPERATOR && node->ivalue == Y_NOT) {
        branch_on_collect(node->left, no, yes, merge, predecessors, result);
    } else if (node->nodeType == BINARY_OPERATOR && node->ivalue == Y_AND) {
        int rhs = ++label;
        branch_on_collect(node->left, rhs, no, merge, predecessors, result);
        start_block(rhs, result);
        branch_on_collect(node->right, yes, no, merge, predecessors, result);
    } else if (node->nodeType == BINARY_OPERATOR && node->ivalue == Y_OR) {
        int rhs = ++label;
        branch_on_collect(node->left, yes, rhs, merge, predecessors, result);
        start_block(rhs, result);
        branch_on_collect(node->right, yes, no, merge, predecessors, result);
    } else {
        genLogicExpr(node, result);
        if (predecessors && (yes == merge || no == merge)) {
            PhiPredecessor *input = (PhiPredecessor *)malloc(sizeof *input);
            if (!input) fail("out of memory");
            input->block = current_block;
            input->next = *predecessors;
            *predecessors = input;
        }
        branch(last_value, yes, no, result);
    }
}

static void branch_on(past node, int yes, int no, char *result)
{
    branch_on_collect(node, yes, no, 0, NULL, result);
}

int genIfStmt(past node, char *result)
{
    int immediate;
    if (constant_truth(node->if_cond, &immediate)) {
        statement(immediate ? node->left : node->right, result);
        return 0;
    }
    int yes = ++label;
    int done = ++label;
    int no = node->right ? ++label : done;
    branch_on(node->if_cond, yes, no, result);
    start_block(yes, result);
    statement(node->left, result);
    int yes_terminated = terminated;
    if (!terminated) jump(done, result);
    if (node->right) {
        start_block(no, result);
        statement(node->right, result);
        if (yes_terminated && terminated) return 0;
        if (!terminated) jump(done, result);
    }
    start_block(done, result);
    return 0;
}

typedef struct {
    int returns;
    int has_break;
    int falls;
} StatementFlow;

/* Breaks are carried through blocks and if statements, then consumed by a loop. */
static StatementFlow statement_flow(past node)
{
    StatementFlow flow = {0, 0, 1};
    for (; node && flow.falls; node = node->next) {
        StatementFlow current = {0, 0, 1};
        switch (node->nodeType) {
        case RETURN_STMT: current.returns = 1; current.falls = 0; break;
        case BREAK_STMT: current.has_break = 1; current.falls = 0; break;
        case CONTINUE_STMT: current.falls = 0; break;
        case COMPOUND_STMT: current = statement_flow(node->left); break;
        case IF_STMT: {
            int truth;
            if (constant_truth(node->if_cond, &truth)) {
                current = statement_flow(truth ? node->left : node->right);
            } else {
                StatementFlow yes = statement_flow(node->left);
                StatementFlow no = statement_flow(node->right);
                current.returns = yes.returns + no.returns;
                current.has_break = yes.has_break || no.has_break;
                current.falls = yes.falls || no.falls;
            }
            break;
        }
        case WHILE_STMT: {
            StatementFlow body = statement_flow(node->right);
            int truth;
            current.returns = body.returns;
            current.falls = !constant_truth(node->left, &truth) || !truth || body.has_break;
            break;
        }
        default: break;
        }
        flow.returns += current.returns;
        flow.has_break |= current.has_break;
        flow.falls = current.falls;
    }
    return flow;
}

int genWhileStmt(past node, char *result)
{
    int test = ++label;
    int body = ++label;
    int done = ++label;
    jump(test, result);
    start_block(test, result);
    int immediate;
    if (!constant_truth(node->left, &immediate) || !immediate) {
        /* Unlike if, while evaluates its condition to an i1 value first.
           Keep logical PHIs and logical-not instructions before the loop branch. */
        genLogicExpr(node->left, result);
        branch(last_value, body, done, result);
        start_block(body, result);
    }
    int old_break = break_target;
    int old_continue = continue_target;
    break_target = done;
    continue_target = test;
    statement(node->right, result);
    break_target = old_break;
    continue_target = old_continue;
    if (sample_output) {
        /* The reference numbers inner loops first and omits metadata definitions. */
        int loop_metadata = next_loop_metadata;
        next_loop_metadata += 2;
        if (!terminated) {
            emit(result, "  br label %%bb%d, !llvm.loop !%d\n", test, loop_metadata);
            terminated = 1;
        }
    } else if (!terminated) {
        jump(test, result);
    }
    if (constant_truth(node->left, &immediate) && immediate && !statement_flow(node->right).has_break) {
        terminated = 1;
        return 0;
    }
    start_block(done, result);
    return 0;
}


static void zero_initialize_tail(Value first, Value next, int count, char *result)
{
    const char *type = value_type(first);
    Value end = fresh(0);
    emit(result, "  %s = getelementptr inbounds %s, %s* %s, i64 %d\n",
         end.operand, type, type, first.operand, count);
    int predecessor = current_block;
    int loop = ++label;
    int done = ++label;
    jump(loop, result);
    start_block(loop, result);
    Value cursor = fresh(0);
    Value advanced = fresh(0);
    if (predecessor == 0)
        emit(result, "  %s = phi %s* [ %s, %%%d ], [ %s, %%bb%d ]\n",
             cursor.operand, type, next.operand, entry_number, advanced.operand, loop);
    else
        emit(result, "  %s = phi %s* [ %s, %%bb%d ], [ %s, %%bb%d ]\n",
             cursor.operand, type, next.operand, predecessor, advanced.operand, loop);
    emit(result, "  store %s %s, %s* %s, align 4\n",
         type, first.is_float ? "0.000000e+00" : "0", type, cursor.operand);
    emit(result, "  %s = getelementptr inbounds %s, %s* %s, i64 1\n",
         advanced.operand, type, type, cursor.operand);
    Value complete = fresh(1);
    emit(result, "  %s = icmp eq %s* %s, %s\n", complete.operand, type, advanced.operand, end.operand);
    branch(complete, done, loop, result);
    start_block(done, result);
}

static int simple_zero(past node)
{
    while (node && node->nodeType == PAREN_EXPR) node = node->left;
    return !node || (node->nodeType == INTEGER_LITERAL && node->ivalue == 0) ||
           (node->nodeType == FLOATING_LITERAL && node->fvalue == 0.0f && !signbit(node->fvalue));
}

static uint64_t explicit_nonzero_elements(past item)
{
    uint64_t count = 0;
    for (; item; item = item->next) {
        if (item->nodeType == INIT_LIST_EXPR) count += explicit_nonzero_elements(item->left);
        else if (!simple_zero(item)) ++count;
    }
    return count;
}

/* Consume one aggregate. Brace groups have their own initializer cursor. */
static int initialize_array(past dimensions, past *item, Value base,
                            int global, int cleared, char *result)
{
    if (global && !*item) {
        emit(result, "zeroinitializer");
        return 1;
    }
    size_t aggregate_start = strlen(result);
    int all_zero = 1;
    if (global) emit(result, "[");
    int size = array_bound(dimensions);
    Value first = {0}, previous = {0};
    for (int i = 0; i < size; ++i) {
        if (cleared && !*item) break;
        if (global && i) emit(result, ", ");
        Value pointer = {.is_float = base.is_float};
        if (!global) {
            if (i == 0) {
                pointer = array_element(base, dimensions, (Value){.operand = "0"}, result);
                first = pointer;
            } else {
                char element_type[TYPE_CAPACITY];
                array_type(dimensions->next, base.is_float, element_type);
                pointer = fresh(0);
                pointer.is_float = base.is_float;
                emit(result, "  %s = getelementptr inbounds %s, %s* %s, i64 1\n",
                     pointer.operand, element_type, element_type, previous.operand);
            }
            pointer.alignment = 4;
            previous = pointer;
            if (!dimensions->next && !*item) {
                zero_initialize_tail(first, pointer, size, result);
                return 0;
            }
        }
        if (dimensions->next) {
            if (global) {
                char type[TYPE_CAPACITY];
                array_type(dimensions->next, base.is_float, type);
                emit(result, "%s ", type);
            }
            if (*item && (*item)->nodeType == INIT_LIST_EXPR) {
                past nested = (*item)->left;
                *item = (*item)->next;
                all_zero &= initialize_array(dimensions->next, &nested, pointer, global, cleared, result);
                if (nested) fail("too many array initializers");
            } else {
                all_zero &= initialize_array(dimensions->next, item, pointer, global, cleared, result);
            }
            continue;
        }

        past scalar = *item;
        if (scalar) *item = scalar->next;
        while (scalar && scalar->nodeType == INIT_LIST_EXPR) {
            scalar = scalar->left;
            if (scalar && scalar->next) fail("too many scalar initializers");
        }
        if (global && base.is_float) {
            float initial;
            if (!constant_float(scalar, &initial)) fail("global array initializer must be constant");
            all_zero &= initial == 0.0f && !signbit(initial);
            Value value = float_value(initial);
            emit(result, "float %s", value.operand);
        } else if (global) {
            int initial = 0;
            if (scalar && !constant_int_initializer(scalar, &initial))
                fail("global array initializer must be constant");
            all_zero &= initial == 0;
            emit(result, "i32 %d", initial);
        } else if (!cleared || !simple_zero(scalar)) {
            Value value = {.operand = "0"};
            if (scalar) value = expression(scalar, result);
            store(value, pointer, result);
        }
    }
    if (global) {
        emit(result, "]");
        if (all_zero) {
            result[aggregate_start] = '\0';
            emit(result, "zeroinitializer");
        }
    }
    return all_zero;
}

static int constant_initializer(past node, int is_float)
{
    for (; node; node = node->next) {
        if (node->nodeType == INIT_LIST_EXPR) {
            if (!constant_initializer(node->left, is_float)) return 0;
        } else if (is_float) {
            float value;
            if (!constant_float(node, &value)) return 0;
        } else {
            int value;
            if (!constant_int_initializer(node, &value)) return 0;
        }
    }
    return 1;
}

static char *array_constant_name(const char *variable)
{
    size_t capacity = strlen(current_function) + strlen(variable) + 48;
    char *name = (char *)malloc(capacity);
    if (!name) fail("out of memory");
    snprintf(name, capacity, "__const.%s.%s", current_function, variable);
    unsigned suffix = 0;
    for (;;) {
        ArrayConstant *existing = array_constants;
        while (existing && strcmp(existing->name, name) != 0) existing = existing->next;
        if (!existing) break;
        snprintf(name, capacity, "__const.%s.%s.%u", current_function, variable, ++suffix);
    }
    ArrayConstant *constant = (ArrayConstant *)malloc(sizeof *constant);
    if (!constant) fail("out of memory");
    constant->name = name;
    constant->next = array_constants;
    array_constants = constant;
    return name;
}

static uint64_t array_bytes(past dimensions)
{
    for (past dim = dimensions; dim; dim = dim->next)
        if (!array_bound(dim)) return 0;
    uint64_t bytes = 4;
    for (past dim = dimensions; dim; dim = dim->next) {
        unsigned size = (unsigned)array_bound(dim);
        if (bytes > UINT64_MAX / size) fail("array is too large");
        bytes *= size;
    }
    return bytes;
}

static void emit_array_memset(Value destination, int alignment, uint64_t bytes, char *result)
{
    require_declaration("llvm.memset.p0i8.i64", "void",
                        "i8* nocapture writeonly, i8, i64, i1 immarg");
    emit(result, "  call void @llvm.memset.p0i8.i64(i8* align %d %s, i8 0, i64 %llu, i1 false)\n",
         alignment, destination.operand, (unsigned long long)bytes);
}

enum { SPARSE_STORE_LIMIT = 6, PACKED_ZERO_TAIL = 8 };

typedef struct {
    uint64_t index;
    Value value;
} SparseElement;

/* Collect the nonzero scalar leaves using the same brace boundaries as the
   regular initializer. Stop once memset plus stores would be too expensive. */
static int collect_sparse_elements(past dim, past *item, int is_float,
                                   uint64_t offset, SparseElement *elements,
                                   int *count, int *packed)
{
    uint64_t width = array_bytes(dim->next) / 4;
    int size = array_bound(dim);
    int first = *count;
    int last_nonzero = -1;
    for (int i = 0; i < size && *item; ++i) {
        int before = *count;
        if (dim->next) {
            past nested = NULL;
            past *input = item;
            if ((*item)->nodeType == INIT_LIST_EXPR) {
                nested = (*item)->left;
                *item = (*item)->next;
                input = &nested;
            }
            if (!collect_sparse_elements(dim->next, input, is_float,
                                         offset + (uint64_t)i * width, elements,
                                         count, packed)) return 0;
            if (nested) fail("too many array initializers");
        } else {
            past scalar = *item;
            *item = scalar->next;
            while (scalar && scalar->nodeType == INIT_LIST_EXPR) {
                scalar = scalar->left;
                if (scalar && scalar->next) fail("too many scalar initializers");
            }
            Value value = {.is_float = is_float};
            if (is_float) {
                float initial;
                if (!constant_float(scalar, &initial)) return 0;
                if (initial == 0.0f && !signbit(initial)) continue;
                value = float_value(initial);
            } else {
                int initial = 0;
                if (scalar && !constant_int_initializer(scalar, &initial)) return 0;
                if (!initial) continue;
                snprintf(value.operand, sizeof value.operand, "%d", initial);
            }
            if (*count == SPARSE_STORE_LIMIT) return 0;
            elements[*count].index = offset + (uint64_t)i;
            elements[(*count)++].value = value;
        }
        if (*count != before) last_nonzero = i;
    }
    if (*count != first && size - last_nonzero - 1 >= PACKED_ZERO_TAIL) *packed = 1;
    return 1;
}

static Value sparse_pointer(Value base, const char *type, int index,
                            uint64_t offset, char *result)
{
    Value pointer = fresh(0);
    pointer.is_float = base.is_float;
    pointer.alignment = base.alignment;
    while (pointer.alignment > 4 && offset % (unsigned)pointer.alignment) pointer.alignment /= 2;
    emit(result, "  %s = getelementptr inbounds %s, %s* %s, i32 0, i32 %d\n",
         pointer.operand, type, type, base.operand, index);
    return pointer;
}

static void emit_sparse_array_stores(past dim, Value base, const char *type,
                                    uint64_t start, const SparseElement *elements,
                                    int count, char *result)
{
    uint64_t width = array_bytes(dim->next) / 4;
    for (int first = 0; first < count; ) {
        uint64_t index = (elements[first].index - start) / width;
        Value pointer = sparse_pointer(base, type, (int)index, index * width * 4, result);
        int next = first + 1;
        if (dim->next) {
            uint64_t offset = start + width * index;
            while (next < count && elements[next].index - offset < width) ++next;
            char inner[TYPE_CAPACITY];
            array_type(dim->next, base.is_float, inner);
            emit_sparse_array_stores(dim->next, pointer, inner, offset, elements + first,
                                     next - first, result);
        } else {
            store(elements[first].value, pointer, result);
        }
        first = next;
    }
}

static int try_sparse_constant_array(past node, Value base, const char *type,
                                     uint64_t bytes, char *result)
{
    if (bytes <= 32) return 0;
    SparseElement elements[SPARSE_STORE_LIMIT];
    int count = 0, packed = 0;
    past item = node->left->left;
    if (!collect_sparse_elements(node->right, &item, node->is_float, 0,
                                 elements, &count, &packed)) return 0;
    if (item) fail("too many array initializers");
    /* Nested packed shapes use the existing aggregate copy. */
    if (packed && node->right->next) return 0;

    Value destination = fresh(0);
    emit(result, "  %s = bitcast %s* %s to i8*\n", destination.operand, type, base.operand);
    emit_array_memset(destination, base.alignment, bytes, result);
    if (!count) return 1;

    char packed_type[TYPE_CAPACITY], prefix_type[TYPE_CAPACITY];
    const char *storage_type = type;
    int prefix = 0;
    if (packed) {
        prefix = (int)elements[count - 1].index + 1;
        const char *scalar = value_type(base);
        int tail = array_bound(node->right) - prefix;
        if (prefix >= PACKED_ZERO_TAIL) {
            snprintf(prefix_type, sizeof prefix_type, "[%d x %s]", prefix, scalar);
            snprintf(packed_type, sizeof packed_type, "<{ [%d x %s], [%d x %s] }>",
                     prefix, scalar, tail, scalar);
        } else {
            size_t used = 0;
            for (int i = 0; i < prefix; ++i)
                used += (size_t)snprintf(packed_type + used, sizeof packed_type - used,
                                         "%s%s, ", i == 0 ? "<{ " : "", scalar);
            snprintf(packed_type + used, sizeof packed_type - used, "[%d x %s] }>", tail, scalar);
        }
        storage_type = packed_type;
    }

    Value aggregate = fresh(0);
    aggregate.is_float = base.is_float;
    aggregate.alignment = base.alignment;
    emit(result, "  %s = bitcast i8* %s to %s*\n", aggregate.operand,
         destination.operand, storage_type);
    if (packed && prefix >= PACKED_ZERO_TAIL) {
        aggregate = sparse_pointer(aggregate, storage_type, 0, 0, result);
        storage_type = prefix_type;
    }
    emit_sparse_array_stores(node->right, aggregate, storage_type, 0, elements, count, result);
    return 1;
}

static void initialize_constant_array(past node, Value base, char *result)
{
    char type[TYPE_CAPACITY];
    array_type(node->right, node->is_float, type);
    uint64_t bytes = array_bytes(node->right);
    if (try_sparse_constant_array(node, base, type, bytes, result)) return;
    int alignment = array_alignment(node->right);
    past item = node->left->left;
    char *data = (char *)calloc(IR_CAPACITY, 1);
    if (!data) fail("out of memory");
    int all_zero = initialize_array(node->right, &item, (Value){.is_float = node->is_float}, 1, 0, data);
    if (item) fail("too many array initializers");

    Value destination = fresh(0);
    emit(result, "  %s = bitcast %s* %s to i8*\n",
         destination.operand, type, base.operand);
    if (bytes == 0) {
        free(data);
        return;
    }
    if (all_zero) {
        emit_array_memset(destination, alignment, bytes, result);
    } else {
        require_declaration("llvm.memcpy.p0i8.p0i8.i64", "void",
                            "i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg");
        const char *name = array_constant_name(node->svalue);
        emit(module_globals, "@%s = private unnamed_addr constant %s %s, align %d\n",
             name, type, data, alignment);
        emit(result, "  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align %d %s, i8* align %d bitcast (%s* @%s to i8*), i64 %llu, i1 false)\n",
             alignment, destination.operand, alignment, type, name, (unsigned long long)bytes);
    }
    free(data);
}

static void declare(past node, char *result)
{
    Binding *binding = push_binding(node);
    snprintf(binding->address.operand, sizeof binding->address.operand,
             "%%%d", node->ivalue);
    binding->address.alignment = node->nodeType == VAR_DECL ? array_alignment(node->right) : 4;
    binding->has_constant = node->is_const && !node->is_float && !binding->dimensions && node->left &&
                            constant_int_initializer(node->left, &binding->constant);

    if (node->nodeType == VAR_DECL && node->left) {
        if (binding->dimensions) {
            if (node->left->nodeType != INIT_LIST_EXPR) fail("expected array initializer list");
            if (constant_initializer(node->left->left, node->is_float)) {
                initialize_constant_array(node, binding->address, result);
            } else {
                past item = node->left->left;
                uint64_t bytes = array_bytes(binding->dimensions);
                int cleared = bytes > 16 && explicit_nonzero_elements(item) <= bytes / 16;
                if (cleared) {
                    char type[TYPE_CAPACITY];
                    array_type(binding->dimensions, node->is_float, type);
                    Value destination = fresh(0);
                    emit(result, "  %s = bitcast %s* %s to i8*\n",
                         destination.operand, type, binding->address.operand);
                    emit_array_memset(destination, binding->address.alignment, bytes, result);
                }
                initialize_array(binding->dimensions, &item, binding->address, 0, cleared, result);
                if (item) fail("too many array initializers");
            }
        } else {
            store(expression(node->left, result), binding->address, result);
        }
    }
}

/* An array in an expression statement decays even when its value is unused. */
static int discard_array_expression(past node, char *result)
{
    while (node->nodeType == PAREN_EXPR) node = node->left;
    int rank = 0;
    past base = node;
    while (base->nodeType == ARRAY_SUBSCRIPT_EXPR) {
        ++rank;
        base = base->left;
    }
    if (base->nodeType != DECL_REF_EXPR) return 0;

    Binding *local = find_local(base->svalue);
    past global = local ? NULL : find_global(base->svalue);
    if (!local && !global) return 0;
    past dimensions = local ? local->dimensions : global->right;
    for (int i = 0; i < rank && dimensions; ++i) dimensions = dimensions->next;
    if (!dimensions) return 0;
    int is_float = local ? local->address.is_float : global->is_float;
    (void)array_argument(node, dimensions->next, is_float, result);
    return 1;
}

static void statement(past node, char *result)
{
    if (!node || terminated) return;
    switch (node->nodeType) {
    case COMPOUND_STMT: {
        Binding *outer = locals;
        for (past item = node->left; item && !terminated; item = item->next)
            statement(item, result);
        pop_bindings(outer);
        break;
    }
    case DECL_STMT:
        for (past var = node->left; var; var = var->next)
            declare(var, result);
        break;
    case NULL_STMT: break;
    case VAR_DECL: declare(node, result); break;
    case IF_STMT: genIfStmt(node, result); break;
    case WHILE_STMT: genWhileStmt(node, result); break;
    case BREAK_STMT:
        if (!break_target) fail("break outside a loop");
        jump(break_target, result);
        break;
    case CONTINUE_STMT:
        if (!continue_target) fail("continue outside a loop");
        jump(continue_target, result);
        break;
    case RETURN_STMT: {
        if (returns_void) {
            if (node->left) fail("void function cannot return a value");
            if (return_target) jump(return_target, result);
            else {
                emit(result, "  ret void\n");
                terminated = 1;
            }
            break;
        }
        Value value = convert_value(expression(node->left, result), returns_float, result);
        if (return_target) {
            store(value, return_slot, result);
            jump(return_target, result);
        } else {
            emit(result, "  ret %s %s\n", value_type(value), value.operand);
            terminated = 1;
        }
        break;
    }
    case BINARY_OPERATOR:
        if (node->ivalue == Y_ASSIGN) {
            genAssignStmt(node, result);
            break;
        }
        /* An expression statement can also have a binary operator. */
        /* fall through */
    default: {
        if (discard_array_expression(node, result)) break;
        Value discarded = expression(node, result);
        /* C logical/comparison expressions have type int even when discarded. */
        if (discarded.boolean) (void)as_int(discarded, result);
        break;
    }
    }
}

/* Allocate local slots once, even when a declaration is inside a loop. */
static void allocate_locals(past node, char *result)
{
    for (; node; node = node->next) {
        if (node->nodeType == VAR_DECL) {
            node->ivalue = ++temporary;
            char type[TYPE_CAPACITY];
            array_type(node->right, node->is_float, type);
            emit(result, "  %%%d = alloca %s, align %d\n",
                 node->ivalue, type, array_alignment(node->right));
        }
        int immediate;
        if (node->nodeType == IF_STMT && constant_truth(node->if_cond, &immediate)) {
            allocate_locals(immediate ? node->left : node->right, result);
            continue;
        }
        allocate_locals(node->left, result);
        allocate_locals(node->right, result);
        allocate_locals(node->if_cond, result);
    }
}

/* LLVM assigns unnamed blocks and temporaries numbers in definition order. */
static void number_blocks(char *result, int return_alias)
{
    int *blocks = (int *)calloc((size_t)label + 1, sizeof *blocks);
    int *values = (int *)calloc((size_t)temporary + 1, sizeof *values);
    char *text = (char *)malloc(strlen(result) + 1);
    if (!blocks || !values || !text) fail("out of memory");
    strcpy(text, result);

    int slot = entry_number;
    for (int i = 0; i <= entry_number; ++i) values[i] = i;
    for (const char *line = text; line; ) {
        if (strncmp(line, "bb", 2) == 0)
            blocks[atoi(line + 2)] = ++slot;
        else if (strncmp(line, "  %", 3) == 0)
            values[atoi(line + 3)] = ++slot;
        line = strchr(line, '\n');
        if (line) ++line;
    }

    if (return_alias) blocks[return_target] = blocks[return_alias];
    result[0] = '\0';
    const char *segment = text;
    for (const char *p = text; *p; ) {
        const char *number = p;
        int is_block = strncmp(p, "bb", 2) == 0 &&
                       p[2] >= '0' && p[2] <= '9' &&
                       (p == text || p[-1] == '%' || p[-1] == '\n');
        int is_value = *p == '%' && p[1] >= '0' && p[1] <= '9';
        if (!is_block && !is_value) {
            ++p;
            continue;
        }
        number += is_block ? 2 : 1;
        char *end;
        long index = strtol(number, &end, 10);
        emit(result, "%.*s%s%d", (int)(p - segment), segment,
             is_value ? "%" : "", is_block ? blocks[index] : values[index]);
        segment = p = end;
    }
    emit(result, "%s", segment);
    free(text);
    free(values);
    free(blocks);
}

static void gen_function(past node, char *result)
{
    result[0] = '\0';
    entry_number = label = break_target = continue_target = 0;
    current_function = node->svalue;
    returns_void = node->ivalue == Y_VOID;
    returns_float = node->is_float;
    emit(result, "define %s @%s(", returns_void ? "void" : returns_float ? "float" : "i32", node->svalue);
    for (past param = node->left; param; param = param->next) {
        char type[TYPE_CAPACITY];
        array_type(param->right ? param->right->next : NULL, param->is_float, type);
        emit(result, "%s%s%s%s %%%d", entry_number ? ", " : "",
             type, param->right ? "*" : "", sample_output ? "" : " noundef", entry_number);
        ++entry_number;
    }
    emit(result, ") {\n");
    temporary = entry_number;
    current_block = terminated = 0;
    StatementFlow flow = statement_flow(node->right);
    return_target = flow.returns + flow.falls > 1 ? ++label : 0;
    int is_main = strcmp(node->svalue, "main") == 0;
    if (!returns_void && (is_main || return_target)) {
        return_slot = fresh(0);
        return_slot.is_float = returns_float;
        emit(result, "  %s = alloca %s, align 4\n", return_slot.operand, value_type(return_slot));
    }

    for (past param = node->left; param; param = param->next) {
        param->ivalue = ++temporary;
        char type[TYPE_CAPACITY];
        array_type(param->right ? param->right->next : NULL, param->is_float, type);
        emit(result, "  %%%d = alloca %s%s, align %d\n", param->ivalue,
             type, param->right ? "*" : "", param->right ? 8 : 4);
    }
    allocate_locals(node->right, result);
    if (is_main && !returns_void)
        emit(result, "  store i32 0, i32* %s, align 4\n", return_slot.operand);
    int index = 0;
    for (past param = node->left; param; param = param->next) {
        declare(param, result);
        char type[TYPE_CAPACITY];
        array_type(param->right ? param->right->next : NULL, param->is_float, type);
        const char *pointer = param->right ? "*" : "";
        emit(result, "  store %s%s %%%d, %s%s* %%%d, align %d\n", type, pointer,
             index++, type, pointer, param->ivalue, param->right ? 8 : 4);
    }
    statement(node->right, result);
    int return_alias = 0;
    if (return_target) {
        size_t length = strlen(result);
        if (!terminated && current_block && length >= 2 && result[length - 2] == ':') {
            return_alias = current_block;
        } else {
            if (!terminated) jump(return_target, result);
            start_block(return_target, result);
        }
        if (returns_void) {
            emit(result, "  ret void\n");
        } else {
            Value value = fresh(0);
            value.is_float = returns_float;
            emit(result, "  %s = load %s, %s* %s, align 4\n",
                 value.operand, value_type(value), value_type(value), return_slot.operand);
            emit(result, "  ret %s %s\n", value_type(value), value.operand);
        }
    } else if (!terminated) {
        emit(result, returns_void ? "  ret void\n" : returns_float ? "  ret float 0.000000e+00\n" : "  ret i32 0\n");
    }
    emit(result, "}\n");
    if (label != 0) number_blocks(result, return_alias);
    pop_bindings(NULL);
}


static int constant_float(past node, float *value)
{
    if (!node) { *value = 0; return 1; }
    if (node->nodeType == FLOATING_LITERAL) { *value = node->fvalue; return 1; }
    int integer;
    if (constant_int(node, &integer)) { *value = (float)integer; return 1; }
    if (node->nodeType == PAREN_EXPR) return constant_float(node->left, value);
    float left, right;
    if (node->nodeType == UNARY_OPERATOR && constant_float(node->left, &left)) {
        if (node->ivalue == Y_ADD) { *value = left; return 1; }
        if (node->ivalue == Y_SUB) { *value = -left; return 1; }
    }
    if (node->nodeType != BINARY_OPERATOR ||
        !constant_float(node->left, &left) || !constant_float(node->right, &right)) return 0;
    switch (node->ivalue) {
    case Y_ADD: *value = left + right; return 1;
    case Y_SUB: *value = left - right; return 1;
    case Y_MUL: *value = left * right; return 1;
    case Y_DIV: *value = left / right; return 1;
    default: return 0;
    }
}

typedef struct {
    char *text;
    size_t length, capacity;
} LayoutText;

typedef struct {
    char *type;
    char *initializer;
    int is_zero;
} ConstantLayout;

static void append_layout_text(LayoutText *out, const char *text)
{
    size_t size = strlen(text);
    if (size >= IR_CAPACITY - out->length) fail("array initializer is too large");
    size_t need = out->length + size + 1;
    if (need > out->capacity) {
        size_t capacity = out->capacity ? out->capacity * 2 : 64;
        if (capacity < need) capacity = need;
        char *next = (char *)realloc(out->text, capacity);
        if (!next) fail("out of memory");
        out->text = next;
        out->capacity = capacity;
    }
    memcpy(out->text + out->length, text, size + 1);
    out->length += size;
}

static char *copy_layout_text(const char *text)
{
    size_t size = strlen(text) + 1;
    char *copy = (char *)malloc(size);
    if (!copy) fail("out of memory");
    memcpy(copy, text, size);
    return copy;
}

static void free_constant_layout(ConstantLayout value)
{
    free(value.type);
    free(value.initializer);
}

static ConstantLayout scalar_layout(past item, int is_float)
{
    while (item && item->nodeType == INIT_LIST_EXPR) {
        item = item->left;
        if (item && item->next) fail("too many scalar initializers");
    }
    Value value = {.is_float = is_float};
    int zero;
    if (is_float) {
        float initial;
        if (!constant_float(item, &initial)) fail("global array initializer must be constant");
        zero = initial == 0.0f && !signbit(initial);
        value = float_value(initial);
    } else {
        int initial = 0;
        if (item && !constant_int_initializer(item, &initial))
            fail("global array initializer must be constant");
        zero = initial == 0;
        snprintf(value.operand, sizeof value.operand, "%d", initial);
    }
    return (ConstantLayout){copy_layout_text(value_type(value)), copy_layout_text(value.operand), zero};
}

static char *layout_array_type(const char *element, int count)
{
    LayoutText out = {0};
    char prefix[32];
    snprintf(prefix, sizeof prefix, "[%d x ", count);
    append_layout_text(&out, prefix);
    append_layout_text(&out, element);
    append_layout_text(&out, "]");
    return out.text;
}

/* Each operand carries its own type. Only uniformly typed operands can use
   the array form; mixed or compacted operands require a packed structure. */
static ConstantLayout render_constant_array(const ConstantLayout *items, int used,
                                            ConstantLayout zero, int size,
                                            const char *uniform_type)
{
    LayoutText data = {0}, packed_type = {0};
    if (!uniform_type) append_layout_text(&packed_type, "<{ ");
    append_layout_text(&data, uniform_type ? "[" : "<{ ");
    for (int i = 0; i < size; ++i) {
        ConstantLayout item = i < used ? items[i] : zero;
        if (i) {
            append_layout_text(&data, ", ");
            if (!uniform_type) append_layout_text(&packed_type, ", ");
        }
        append_layout_text(&data, item.type);
        append_layout_text(&data, " ");
        append_layout_text(&data, item.initializer);
        if (!uniform_type) append_layout_text(&packed_type, item.type);
    }
    append_layout_text(&data, uniform_type ? "]" : " }>");
    if (!uniform_type) append_layout_text(&packed_type, " }>");
    return (ConstantLayout){uniform_type ? layout_array_type(uniform_type, size) : packed_type.text,
                            data.text, 0};
}

static ConstantLayout build_constant_layout(past dim, past *item, int is_float)
{
    int bound = array_bound(dim);
    int used = 0, capacity = 0, nonzero = 0;
    ConstantLayout *parts = NULL;
    const char *uniform_type = NULL;
    for (; used < bound && *item; ++used) {
        ConstantLayout part;
        if (dim->next) {
            if ((*item)->nodeType == INIT_LIST_EXPR) {
                past nested = (*item)->left;
                *item = (*item)->next;
                part = build_constant_layout(dim->next, &nested, is_float);
                if (nested) fail("too many array initializers");
            } else {
                part = build_constant_layout(dim->next, item, is_float);
            }
        } else {
            part = scalar_layout(*item, is_float);
            *item = (*item)->next;
        }
        if (used == capacity) {
            int next_capacity = capacity == 0 ? (bound < 8 ? bound : 8)
                : (capacity > bound / 2 ? bound : capacity * 2);
            if ((size_t)next_capacity > SIZE_MAX / sizeof *parts) fail("array initializer is too large");
            ConstantLayout *next = (ConstantLayout *)realloc(parts, (size_t)next_capacity * sizeof *parts);
            if (!next) fail("out of memory");
            parts = next;
            capacity = next_capacity;
        }
        parts[used] = part;
        if (used == 0) uniform_type = part.type;
        else if (uniform_type && strcmp(uniform_type, part.type) != 0) uniform_type = NULL;
        if (!part.is_zero) nonzero = used + 1;
    }

    char natural[TYPE_CAPACITY];
    array_type(dim->next, is_float, natural);
    ConstantLayout zero = dim->next
        ? (ConstantLayout){copy_layout_text(natural), copy_layout_text("zeroinitializer"), 1}
        : scalar_layout(NULL, is_float);
    ConstantLayout value;
    int trailing = bound - nonzero;
    if (nonzero == 0) {
        value = (ConstantLayout){layout_array_type(natural, bound), copy_layout_text("zeroinitializer"), 1};
    } else if (trailing >= PACKED_ZERO_TAIL) {
        ConstantLayout tail = {layout_array_type(uniform_type ? uniform_type : natural, trailing),
                               copy_layout_text("zeroinitializer"), 1};
        if (uniform_type && nonzero >= PACKED_ZERO_TAIL) {
            ConstantLayout chunks[] = {render_constant_array(parts, nonzero, zero, nonzero, uniform_type), tail};
            value = render_constant_array(chunks, 2, zero, 2, NULL);
            free_constant_layout(chunks[0]);
        } else {
            /* Reuse one initialized slot for the filler while rendering. */
            if (nonzero == used) {
                ConstantLayout *next = (ConstantLayout *)realloc(parts, ((size_t)used + 1) * sizeof *parts);
                if (!next) fail("out of memory");
                parts = next;
            }
            ConstantLayout saved = nonzero < used ? parts[nonzero] : (ConstantLayout){0};
            parts[nonzero] = tail;
            value = render_constant_array(parts, nonzero + 1, zero, nonzero + 1, NULL);
            parts[nonzero] = saved;
        }
        free_constant_layout(tail);
    } else {
        if (used < bound && uniform_type && strcmp(zero.type, uniform_type) != 0) uniform_type = NULL;
        value = render_constant_array(parts, used, zero, bound, uniform_type);
    }
    for (int i = 0; i < used; ++i) free_constant_layout(parts[i]);
    free(parts);
    free_constant_layout(zero);
    return value;
}

/* Initialized globals are emitted at their definition. Tentative globals
   take their first-use position, with unused ones emitted last. */
static void emit_global_definition(past global)
{
    if (global->ivalue || !module_globals) return;
    global->ivalue = 1;
    char *result = module_globals;
    if (global->is_float && !global->right) {
        float initial;
        if (!constant_float(global->left, &initial)) fail("expected a constant float initializer");
        Value value = float_value(initial);
        emit(result, "@%s = %s float %s, align 4\n", global->svalue,
             global->is_const ? "constant" : "global", value.operand);
        return;
    }
    if (global->right) {
        char type[TYPE_CAPACITY];
        array_type(global->right, global->is_float, type);
        const char *keyword = global->is_const ? "constant" : "global";
        int alignment = array_alignment(global->right);
        if (!global->left) {
            emit(result, "@%s = %s %s zeroinitializer, align %d\n",
                 global->svalue, keyword, type, alignment);
            return;
        }
        if (global->left->nodeType != INIT_LIST_EXPR) fail("expected array initializer list");
        past item = global->left->left;
        ConstantLayout value = build_constant_layout(global->right, &item, global->is_float);
        if (item) fail("too many array initializers");
        emit(result, "@%s = %s %s %s, align %d\n",
             global->svalue, keyword, value.type, value.initializer, alignment);
        if (strcmp(type, value.type) != 0) {
            GlobalArrayLayout *layout = (GlobalArrayLayout *)malloc(sizeof *layout);
            if (!layout) fail("out of memory");
            layout->declaration = global;
            layout->storage_type = value.type;
            layout->next = global_array_layouts;
            global_array_layouts = layout;
            value.type = NULL;
        }
        free_constant_layout(value);
        return;
    }
    int initial = 0;
    if (global->left && !constant_int_initializer(global->left, &initial))
        fail("global initializer must be a constant integer expression");
    emit(result, "@%s = %s i32 %d, align 4\n",
         global->svalue, global->is_const ? "constant" : "global", initial);
}

static void generate_program(past node, char *result, int sample)
{
    if (!node) fail("expected an int function definition");

    sample_output = sample;
    next_loop_metadata = 6;

    char *body = (char *)malloc(IR_CAPACITY);
    char *function_bodies = (char *)calloc(IR_CAPACITY, 1);
    if (!body || !function_bodies) fail("out of memory");
    resolve_constants(node, 1);
    pop_bindings(NULL);
    functions = node;
    result[0] = '\0';
    module_globals = result;
    /* Global VAR_DECL values use ivalue as an emission flag; locals use stack slots. */
    for (past global = node; global; global = global->next)
        if (global->nodeType == VAR_DECL) global->ivalue = 0;
    int emitted_function = 0;
    for (past function = node; function; function = function->next) {
        if (function->nodeType == VAR_DECL && function->left) emit_global_definition(function);
        if (function->nodeType != FUNCTION_DECL) continue;
        gen_function(function, body);
        if (emitted_function && !sample_output) emit(function_bodies, "\n");
        emitted_function = 1;
        emit(function_bodies, "%s", body);
        emit_declarations(function_bodies);
    }
    for (past global = node; global; global = global->next)
        if (global->nodeType == VAR_DECL) emit_global_definition(global);
    emit(result, "%s", function_bodies);
    free_declarations();
    while (array_constants) {
        ArrayConstant *constant = array_constants;
        array_constants = constant->next;
        free(constant->name);
        free(constant);
    }
    while (global_array_layouts) {
        GlobalArrayLayout *layout = global_array_layouts;
        global_array_layouts = layout->next;
        free(layout->storage_type);
        free(layout);
    }
    functions = NULL;
    current_function = NULL;
    module_globals = NULL;
    free(function_bodies);
    free(body);
    sample_output = 0;
}

void genProgram(past node, char *result)
{
    generate_program(node, result, 0);
}

void genSampleProgram(past node, char *result)
{
    generate_program(node, result, 1);
}










