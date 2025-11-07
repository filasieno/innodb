
/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

// Helper function to generate binary expressions
function op($, operator, precedence) {
  return prec.left(precedence, seq(
    field('lhs_expr', $.cexpr),
    operator,
    field('rhs_expr', $.cexpr)
  ));
}

// Helper function to generate binary expressions with choice of operators
function op_choice($, operators, precedence) {
  return prec.left(precedence, seq(
    field('lhs_expr', $.cexpr),
    choice(...operators),
    field('rhs_expr', $.cexpr)
  ));
}

module.exports = grammar({
  name: "cps",

  supertypes: $ => [
    $.top_level_def,
    $.astmt,         // atomic  statement
    $.cstmt,         // complex statement
    $.type,          // type
  ],

  rules: {
    file: $ => repeat(
      field("top_level_def", $.top_level_def)
    ),
    
    // top level definitions
    // ------------------------------------------------------------------------------------------------------------

    top_level_def: $ => choice(
      $.async_fun_def,
      $.fun_def,
      $.coro_def,
      // $.struct_def,
      // $.union_def,
      // $.enum_def,
      $.namespace_def,
      $.set_namespace_def,
    ),

    async_fun_def: $ => seq(
      'tail', 'fn',
      field('name', $.identifier),
      '(',
      optional(field('params', $.parameter_list)),
      ')',
      field('return_types', $.type_list),
      field('body', $.block)
    ),

    fun_def: $ => seq(
      'fn',
      field('name', $.identifier),
      '(',
      optional(field('params', $.parameter_list)),
      ')',
      field('return_types', $.type_list),
      field('body', $.block)
    ),

    coro_def: $ => seq(
      'coro',
      field('name', $.identifier),
      '(',
      optional(field('params', $.parameter_list)),
      ')',
      field('return_types', $.type_list),
      field('body', $.block)
    ),
    parameter: $ => seq(
      field('type', $.type),
      field('name',$.identifier),
    ),

    parameter_list: $ => seq($.parameter, repeat(seq(',', $.parameter))),
    
    type_list: $ => seq($.type, repeat(seq(',', $.type))),

    namespace_def: $ => seq(
      seq('namespace', 
        field('name', $.identifier)),
        '{', 
        field("defs", repeat($.top_level_def)),
        '}'
    ),

    set_namespace_def: $ => seq(
      'namespace', $.identifier, ';'
    ),
    
    // types
    // ------------------------------------------------------------------------------------------------------------

    type: $ => choice(
      $.named_type,
      $.pointer_type,
      $.fixed_size_array_type,
      $.coro_fn_type,
      $.fn_type,
      $.tail_fn_type,
    ),
    named_type:            $ => field("name", $.identifier),
    pointer_type:          $ => seq('*', field("name", $.identifier)),
    fixed_size_array_type: $ => seq('[', field("size", $.integer_literal), ']', field("name", $.identifier)),
    coro_fn_type:          $ => seq('coro', '(', field("params", $.type_param_list) ,')', field("returns", $.type)),    
    fn_type:               $ => seq('fn', '(', field("params", $.type_param_list) ,')', field("returns", $.type)),    
    tail_fn_type:          $ => seq('tail', 'fn', '(', field("params", $.type_param_list) ,')', field("returns", $.type)),
    type_param_list:       $ => seq($.type, repeat(seq(',', $.type))),
    
    // atomic statements
    // ------------------------------------------------------------------------------------------------------------

    astmt: $ => choice(
      $.var_definition_stmt,
      $.let_definition_stmt,
      $.assignment_stmt,
      $.return_stmt,
    ),
    var_definition_stmt: $ => seq('var', field('name', $.identifier), field('type', $.type), '=', field('expr', $.cexpr)),
    let_definition_stmt: $ => seq('let', field('name', $.identifier), field('type', $.type), '=', field('expr', $.cexpr)),
    assignment_stmt:     $ => seq(field('name', $.identifier), field("operator", choice('=','+=', '-=', '*=', '/=', '%=', '<<=', '>>=', '&=', '^=', '|=')), field('value', $.cexpr),),
    return_stmt:         $ => seq('return', optional(field('expr', $.cexpr))),
    continue_stmt:       $ => seq('continue'),
    break_stmt:          $ => seq('break'),
    suspend_stmt:        $ => seq('suspend'),
    goto_stmt:           $ => seq('goto', field('label', $.identifier)),
    labeled_stmt:        $ => seq(':', field('label', $.identifier), field('stmt', $.cstmt)),

    // complex statements
    // ------------------------------------------------------------------------------------------------------------
    cstmt: $ => choice(
      $.astmt,
      $.if_stmt,
      $.while_stmt,
      $.do_while_stmt,
      $.for_stmt,
    ),
    
    if_stmt: $ => prec.right(seq(
      'if',
      '(',
      field('cond_expr', $.cexpr),
      ')',
      field('true_stmt', $.cstmt),
      optional(seq('else', field('false_stmt', $.cstmt)))
    )),

    while_stmt: $ => seq(
      'while',
      '(',
      field('condition', $.cexpr),
      ')',
      field('body', $.cstmt)
    ),

    do_while_stmt: $ => seq(
      'do',
      field('body', $.cstmt),
      'while',
      '(',
      field('condition', $.cexpr),
      ')',
    ),

    for_stmt: $ => seq(
      'for', '(', 
      field('initializer', optional($.astmt)), ';', 
      field('condition',   optional($.cexpr)), ';', 
      field('update',      optional($.astmt)), ';',
      ')',
      field('body', $.cstmt)
    ),

    block: $ => seq(
      '{',
      field('stmts', optional($.stmt_list)), 
      '}'
    ),
    stmt_list: $ =>seq($.cstmt, repeat(seq(';', $.cstmt))),

    // axpr - Atomic expressions
    // ------------------------------------------------------------------------------------------------------------
    expr: $ => choice(

    ),

    aexpr: $ => choice(
      $.float_literal,
      $.binary_literal,
      $.hex_literal,
      $.octal_literal,
      $.boolean_literal,
      $.null_literal,
      $.identifier,
    ),
    float_literal:   $ => /\d+\.\d+([eE][+-]?\d+)?[fFlL]?|\d+[eE][+-]?\d+[fFlL]?/,
    binary_literal:  $ => /0[bB][01]+[uUlL]?/,
    hex_literal:     $ => /0[xX][0-9a-fA-F]+[uUlL]?/,
    octal_literal:   $ => /0[0-7]+[uUlL]?/,
    boolean_literal: $ => choice('true', 'false'),
    integer_literal: $ => /[1-9][_0-9]*/,
    null_literal:    $ => 'null',
    identifier:      $ => field("name", /[a-zA-Z_][a-zA-Z0-9_]*/),
    
    // expr - Call Expr 
    sexpr: $ => seq(
      $.aexpr,
      $.function_call,
      // $.member_call,
    ),

    function_call: $ => seq(field("fn", $.cexpr), '(', field('arguments', optional($.argument_list)), ')'),      
    // member_call:   $ => seq(field("fn", $.cexpr), ':', '(', field('arguments', optional($.argument_list)), ')'),
    
    argument_list: $ => seq($.cexpr, repeat(seq(',', $.cexpr))),

    field_access:  $ => seq(field("expr", $.cexpr), '.', field('member', $.identifier)),
      
    // cxpr - Complex expressions
    // ------------------------------------------------------------------------------------------------------------
    cexpr: $ => choice(
      $.sexpr,
      // $.unary_expr,
      $._binary_expr,
      // $.member_call,
      $.parentesized_expr,
    ),
    parentesized_expr: $ => prec(200, seq('(', $.cexpr, ')')),
    
    // unary_expr: $ => choice(
    //   prec.right(100, seq('!', field('expr', $.cexpr))),
    //   prec.right(100, seq('~', field('expr', $.cexpr))),
    // ),

    // cxpr - Complex expressions
    // ------------------------------------------------------------------------------------------------------------
    _binary_expr: $ => choice(
      // C binary expressions: logical, bitwise, equality, relational, shift, additive, multiplicative
      // Using helper functions for cleaner, more maintainable code
      op($, '||', 4),  // Logical OR
      op($, '&&', 5),  // Logical AND
      op($, '|', 6),   // Bitwise OR
      op($, '^', 7),   // Bitwise XOR
      op($, '&', 8),   // Bitwise AND
      op_choice($, ['==', '!='], 9),            // Equality
      op_choice($, ['<', '>', '<=', '>='], 10), // Relational
      op_choice($, ['<<', '>>'], 11),           // Shift
      op_choice($, ['+', '-'], 12),             // Additive
      op_choice($, ['*', '/', '%'], 13),        // Multiplicative
    ),

    
  } 
});

