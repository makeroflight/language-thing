#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "vm.h"

void push(Stack *s, Value val) {
    s->head &= s->size - 1;
    s->at[s->head++] = val;
#ifdef DEBUG_STACK
    if (val.type == VAL_TYPE_DOUBLE) {
        printf("pushing double '%lf'. stack head now at %d\n", val.as.real, s->head);
    } else if (val.type == VAL_TYPE_INTEGER) {
        printf("pushing int '%ld'. stack head now at %d\n", val.as.integer, s->head);
    } else if (val.type == VAL_TYPE_STRING) {
        printf("pushing string '%s'. stack head now at %d\n", val.as.string, s->head);
    }
#endif
}

Value pop(Stack *s) {
    s->head &= s->size - 1;
    s->head--;

#ifdef DEBUG_STACK
    Value val = s->at[s->head];
    if (val.type == VAL_TYPE_DOUBLE) {
        printf("popping double '%lf'. stack head now at %d\n", val.as.real, s->head);
    } else if (val.type == VAL_TYPE_INTEGER) {
        printf("popping int '%ld'. stack head now at %d\n", val.as.integer, s->head);
    } else if (val.type == VAL_TYPE_STRING) {
        printf("popping string '%s'. stack head now at %d\n", val.as.string, s->head);
    }

    return val;
#else
    return s->at[s->head];
#endif
}

/* opcodes */
static void op_add(Stack *s) {
    Value a = pop(s);
    Value b = pop(s);

#ifdef DEBUG_STACK
    if (a.type == VAL_TYPE_INTEGER) {
        printf("value a is of type int\n");
    } else if (a.type == VAL_TYPE_DOUBLE) {
        printf("value a is of type double: %lf\n", a.as.real);
    }

    if (b.type == VAL_TYPE_INTEGER) {
        printf("value b is of type int\n");
    } else if (b.type == VAL_TYPE_DOUBLE) {
        printf("value b is of type double: %lf\n", b.as.real);
    }
#endif /* DEBUG_STACK */

    if (a.type == VAL_TYPE_DOUBLE && b.type == VAL_TYPE_DOUBLE) {
        push(s, double_value(a.as.real + b.as.real));
    } else if (a.type == VAL_TYPE_INTEGER && b.type == VAL_TYPE_INTEGER) {
        push(s, int_value(a.as.integer + b.as.integer));
    } else if (a.type == VAL_TYPE_STRING && b.type == VAL_TYPE_STRING) {
        push(s, add_strings(&a, &b));
    } else {
        // error
    }
}

static void op_sub(Stack *s) {
    Value a = pop(s);
    Value b = pop(s);

    if (a.type == VAL_TYPE_DOUBLE && b.type == VAL_TYPE_DOUBLE) {
        push(s, double_value(b.as.real - a.as.real));
    } else if (a.type == VAL_TYPE_INTEGER && b.type == VAL_TYPE_INTEGER) {
        push(s, int_value(b.as.integer - a.as.integer));
    } else {
        // error
    }
}

static void op_mult(Stack *s) {
    Value a = pop(s);
    Value b = pop(s);

    if (a.type == VAL_TYPE_DOUBLE && b.type == VAL_TYPE_DOUBLE) {
        push(s, double_value(a.as.real * b.as.real));
    } else if (a.type == VAL_TYPE_INTEGER && b.type == VAL_TYPE_INTEGER) {
        push(s, int_value(a.as.integer * b.as.integer));
    } else {
        // error
    }
}

static void op_div(Stack *s) {
    Value a = pop(s);
    Value b = pop(s);

    if (a.type == VAL_TYPE_DOUBLE && b.type == VAL_TYPE_DOUBLE) {
        push(s, double_value(b.as.real / a.as.real));
    } else if (a.type == VAL_TYPE_INTEGER && b.type == VAL_TYPE_INTEGER) {
        push(s, int_value(b.as.integer / a.as.integer));
    } else {
        // error
    }
}

static void op_cmp(Stack *s) {
    Value b = pop(s);
    Value a = pop(s);

    if (a.type == VAL_TYPE_DOUBLE && b.type == VAL_TYPE_DOUBLE) {
        push(s, bool_value(b.as.real == a.as.real));
    } else if (a.type == VAL_TYPE_STRING && b.type == VAL_TYPE_STRING) {
        push(s, bool_value(strcmp(a.as.string, b.as.string) == 0));
    } else {
        report_error("TypeError", "Incompatible types for '=='");
        // error
    }
}


static Stack initialize_stack() {
    Stack stack;
    stack.at = malloc((sizeof *stack.at) * STACK_SIZE);
    stack.head = 0;
    stack.size = STACK_SIZE;

    return stack;
}

static void free_stack(Stack *s) {
    free(s->at);
    s->at = NULL;
}

VirtualMachine initialize_vm() {
    VirtualMachine vm;
    vm.stack = initialize_stack();
    vm.names = create_name_dynarray();
    vm.env = init_table();
    vm.ip = 0;
    vm.state = 0;
    return vm;
}

uint8_t next(uint8_t *ip) {
    return *(++ip);
}

void evaluate(VirtualMachine *vm, BytecodeArray *bytecode) {
    uint8_t *ip = bytecode->array;
    unsigned int size = bytecode->elements;
    Entry *var;

    for (int i = 0; i < size; i++) {
        switch (bytecode->array[i]) {
        case OP_RETURN:
            break;
        case OP_CONSTANT:
            push(&vm->stack, bytecode->constants->array[bytecode->array[i + 1]]);
            i++;
            break;
        case OP_GET_GLOBAL:
            var = get_entry(vm->env, vm->names.array[bytecode->array[i + 1]]);
            if (var != NULL) {
                push(&vm->stack, get_entry(vm->env, vm->names.array[bytecode->array[i + 1]])->value);
            } else {
                report_error("RuntimeError",
                        "variable '%s' not found",
                        vm->names.array[bytecode->array[i+1]]);
                break;
            }
            i++;
            break;
        case OP_SET_GLOBAL:
            add_entry(vm->env, vm->names.array[bytecode->array[i + 1]], pop(&vm->stack));
            i++;
            break;
        case OP_UPDATE_GLOBAL:
            var = get_entry(vm->env, vm->names.array[bytecode->array[i + 1]]);
            if (var != NULL) {
                var->value = pop(&vm->stack);
            } else {
                report_error("RuntimeError",
                        "variable '%s' not found",
                        vm->names.array[bytecode->array[i+1]]);
                break;
            }
            i++;
            break;
        case OP_ADD:
            op_add(&vm->stack);
            break;
        case OP_SUB:
            op_sub(&vm->stack);
            break;
        case OP_MULT:
            op_mult(&vm->stack);
            break;
        case OP_DIV:
            op_div(&vm->stack);
            break;
        case OP_CMP:
            op_cmp(&vm->stack);
            break;
        default:
            printf("unknown instruction\n");
        }
    }
}

unsigned int execute(VirtualMachine *vm, BytecodeArray *bytecode) {
    if (vm == NULL || bytecode == NULL || bytecode->array == NULL) {
        return 0;
    }

    evaluate(vm, bytecode);

    if (vm->stack.head != 0) {
        Value v = pop(&vm->stack);
        print_value(&v);
        printf("\n");
    }
#ifdef DEBUG_TABLE
    printf("--- contents of env ---\n");
    print_table(vm->env);
#endif /* DEBUG_TABLE */
    return 1;
}

void free_vm(VirtualMachine *vm) {
    free_stack(&vm->stack);
    free_name_dynarray(&vm->names);
    free_table(vm->env);
}

#ifdef DEBUG_STACK
void print_stack(VirtualMachine *vm) {
    unsigned int top = vm->stack.head;
    Value *v = vm->stack.at;

    fputs("--- Contents of stack ---\n", stdout);
    for (int i = 0; i < top; i++) {
        printf("%02x: ", i);
        print_value(v++);
        printf("\n");
    }
}
#endif /* DEBUG_STACK */
