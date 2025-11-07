# Proto Robio

## Examples

```text

namespace ib;



namespace x {
  
  fn sum(a int, b int) int, err {

  }

}

fn myproc() {
  x int, y int = sum(10, 20);
  for (int i = 0; i < 100; ++i) {

  }
  
  while (i < 0) {}
  
  do {} while (i < 10);
  
  if (int a = 1, int b = 100; a < 10) { }

  switch x of {
    case 10 {} 
    case 20 {}
    case 30 {}
    else {}
  }

  10:sum(20);
  sum(10,20);
  &ptr;
  ptr->x = 10;
  non_ptr.x = 10;

}  
tail fn sample() {}
coro sample() int | err {
  send_message();
  suspend;
  defer send();
  errdefer x;
  var x *int = try sample();
  suspend;
  resume_next();
}

coro main(*[]string argv) {
  var a *point, b *point = vec2(10, 20, 30, 40);
  let x int = 10;
  a->x = 10;
  var point *point_t;
  point:init(10,20);
  point:print();
  point:transform(10,20);
  point:fini();
  
  u" "
  r" "
  b" "
  """asjaghsha h
  
  """
  vat a = < EOF
    asjaghsha h
    EOF


  var a string =< """
    pippo va in montagna
    aaa
  """;
  
  ⟨ continue the implementation ⟩
}

  
  


⟨ continue the implementation ⟩= {

}

⟨ continue the implementation ⟩+ {
    
}



struct sample : other {
  int x;
}

union sample : other {
  int x;
}




```

## Algorithms 

### Type Checking

- TreeSitter
- [Scope graphs](https://pl.ewi.tudelft.nl/research/projects/scope-graphs/)

### Compiler passes

- AST to IR conversion
  - [Basic block](https://en.wikipedia.org/wiki/Basic_block)
  - [SSA translation phi-function](https://en.wikipedia.org/wiki/Static_single-assignment_form)
- Liveness analisys
- [CPS conversion](https://matt.might.net/articles/cps-conversion/)
- IR LLVM translation

### References

- [Basic program analisys](https://www.cs.columbia.edu/~suman/secure_sw_devel/Basic_Program_Analysis_CF.pdf)
- [Compiler in Java](https://eden.dei.uc.pt/~amilcar/pdf/CompilerInJava.pdf)

