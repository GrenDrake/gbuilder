#include "gbuilder.h"


SymbolDef* SymbolTable::get(const std::string &name) {
    if (symbols.count(name) > 0) {
        return symbols.at(name);
        }

    if (parent) {
        return parent->get(name);
    } else {
        return nullptr;
    }
}

bool SymbolTable::exists(const std::string &name) const {
    if (symbols.count(name) > 0) {
        return true;
    }
    if (parent) {
        return parent->exists(name);
    } else {
        return false;
    }
}

void SymbolTable::add(SymbolDef *symbol, bool functionScope) {
    if (exists(symbol->name)) {
        return;
    }

    if (functionScope && parent) {
        SymbolTable *cur = this;
        while (cur->parent->parent != nullptr) {
            cur = cur->parent;
        }
        cur->add(symbol, false);
    } else {
        symbols.insert({symbol->name, symbol});
    }
}
