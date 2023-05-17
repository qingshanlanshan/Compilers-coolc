#ifndef SEMANT_H_
#define SEMANT_H_

#include <assert.h>
#include <iostream>
#include "cool-tree.h"
#include "stringtab.h"
#include "symtab.h"
#include "list.h"
#include <map>

#define TRUE 1
#define FALSE 0

class ClassTable;
typedef ClassTable *ClassTableP;

// This is a structure that may be used to contain the semantic
// information such as the inheritance graph.  You may use it or not as
// you like: it is only here to provide a container for the supplied
// methods.

class ClassTable
{
private:
    int semant_errors;
    void install_basic_classes();
    ostream &error_stream;

public:
    ClassTable(Classes);
    int errors() { return semant_errors; }
    ostream &semant_error();
    ostream &semant_error(Class_ c);
    ostream &semant_error(Symbol filename, tree_node *t);

    std::map<Symbol, Class_> class_table;
    SymbolTable<Symbol, Symbol> *object_table;
    SymbolTable<Symbol, Symbol> *method_table;
    bool inheritance_cycles;
    bool invalid_parent;

    bool check_inherit_cycle();
    bool check_valid_parents();
    void check_feature();
};
ClassTable *classtable;
method_class* find_method(Symbol class_name, Symbol method_name);
bool type_check(Symbol child, Symbol parent);
attr_class *get_attr(Symbol class_name, Symbol attr_name);
Symbol find_common_ancestor(Symbol, Symbol);
#endif
