# CPSC Compiler

Lets write a plan to create a new programing language compiler.

Unlike other compilers it will be developed as an incremental compiler designed around the *language server protocol*.

The idea is to have build a new way of building software:

- no git: just a relational database which will be designed specifically to support the compiler (ARIES + MVCC: similar to innodb)
- lsp protocol as interface to the compiler (the standard compilation is still supported by filling the DB and the requesting a compilation)
- not based around files which is very slow
- build system designed around PIE (check paper by Eelco Visser)

It will be designed around *coroutines* and *a relational database*.



Physical layout: 

- `xinnodb/include`: 
- `xinnodb/src`: will contain modules
  - `xinnodb/src/cpscmain`:
  - `xinnodb/src/defs`: defines common definitions (already implemented)
  - `xinnodb/src/cpsc-types`: defines common definitions (already implemented)
  - `xinnodb/src/jsonrpc`: will implement the raw jsonrpc validation using metadata offered by the LSP specification.
  - `xinnodb/src/lspcheck`: validate all LSP message and post them to the msgbus
  - `xinnodb/src/worker`: will define workers used to execute jobs; the whole system will be jobified
  - `xinnodb/src/msgbus`: defines a msgbus used to exchange messages to clients and workers
  - `xinnodb/src/lspdoc`: LSP feature: used to syncronize client and server documents
  - `xinnodb/src/lspnb`: LSP feature: used to syncronize client and server notebooks and implement specific features of notebooks
  - `xinnodb/src/lsp/lspsem`: LSP feature: implements the semantic
  - `xinnodb/src/lsp/lspcompl`
  - `xinnodb/src/lsp/client`: will contain implementations

Dependencies:
 - `c`