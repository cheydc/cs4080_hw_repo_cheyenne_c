package com.craftinginterpreters.lox;

import java.util.List;

class LoxFunction implements LoxCallable {
  private final String name;
  private final List<Token> params;
  private final List<Stmt> body;
  private final Environment closure;
  private final boolean isInitializer;
  private final boolean isGetter;

  LoxFunction(Stmt.Function declaration, Environment closure, boolean isInitializer) {
    this.name = declaration.name.lexeme;
    this.params = declaration.params;
    this.body = declaration.body;
    this.closure = closure;
    this.isInitializer = isInitializer;
    this.isGetter = declaration.isGetter;
  }

  LoxFunction(Expr.Function function, Environment closure) {
    this.name = null;
    this.params = function.params;
    this.body = function.body;
    this.closure = closure;
    this.isInitializer = false;
    this.isGetter = false;
  }

  private LoxFunction(String name, List<Token> params, List<Stmt> body,
                      Environment closure, boolean isInitializer, boolean isGetter) {
    this.name = name;
    this.params = params;
    this.body = body;
    this.closure = closure;
    this.isInitializer = isInitializer;
    this.isGetter = isGetter;
  }

  LoxFunction bind(LoxInstance instance) {
    Environment environment = new Environment(closure);
    environment.define("this", instance);
    return new LoxFunction(name, params, body, environment, isInitializer, isGetter);
  }

  boolean isGetter() {
    return isGetter;
  }

  @Override
  public int arity() {
    return params.size();
  }

  @Override
  public Object call(Interpreter interpreter, List<Object> arguments) {
    Environment environment = new Environment(closure);
    for (int i = 0; i < params.size(); i++) {
      environment.define(params.get(i).lexeme, arguments.get(i));
    }

    try {
      interpreter.executeBlock(body, environment);
    } catch (Return returnValue) {
      if (isInitializer) return closure.getAt(0, "this");
      return returnValue.value;
    }

    if (isInitializer) return closure.getAt(0, "this");
    return null;
  }

  @Override
  public String toString() {
    if (name == null) return "<fn>";
    return "<fn " + name + ">";
  }
}