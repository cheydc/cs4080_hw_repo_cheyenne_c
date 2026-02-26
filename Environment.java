// Environment.java
package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

class Environment {
  final Environment enclosing;

  private final Map<String, Object> values = new HashMap<>();

  // Sentinel to represent "declared but not initialized".
  static final Object UNINITIALIZED = new Object();

  Environment() {
    enclosing = null;
  }

  Environment(Environment enclosing) {
    this.enclosing = enclosing;
  }

  void define(String name, Object value) {
    values.put(name, value);
  }

  Object get(Token name) {
    if (values.containsKey(name.lexeme)) {
      Object value = values.get(name.lexeme);
      if (value == UNINITIALIZED) {
        throw new RuntimeError(name, "Variable '" + name.lexeme + "' has not been initialized.");
      }
      return value;
    }

    if (enclosing != null) return enclosing.get(name);

    throw new RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
  }

  void assign(Token name, Object value) {
    if (values.containsKey(name.lexeme)) {
      values.put(name.lexeme, value);
      return;
    }

    if (enclosing != null) {
      enclosing.assign(name, value);
      return;
    }

    throw new RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
  }

  Object getAt(int distance, String name) {
    Object value = ancestor(distance).values.get(name);
    if (value == UNINITIALIZED) {
      // No Token here, but we can still report a runtime error.
      throw new RuntimeError(new Token(TokenType.IDENTIFIER, name, null, -1),
          "Variable '" + name + "' has not been initialized.");
    }
    return value;
  }

  void assignAt(int distance, Token name, Object value) {
    ancestor(distance).values.put(name.lexeme, value);
  }

  private Environment ancestor(int distance) {
    Environment environment = this;
    for (int i = 0; i < distance; i++) {
      environment = environment.enclosing;
    }
    return environment;
  }
}