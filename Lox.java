// Lox.java
private static void runPrompt() {
  InputStreamReader input = new InputStreamReader(System.in);
  BufferedReader reader = new BufferedReader(input);

  for (;;) {
    try {
      System.out.print("> ");
      String line = reader.readLine();
      if (line == null) break;
      run(line, true);     // true = repl mode
      hadError = false;    // keep REPL alive after an error
    } catch (IOException e) {
      System.out.println(e.getMessage());
      break;
    }
  }
}

private static void run(String source, boolean repl) {
  Scanner scanner = new Scanner(source);
  List<Token> tokens = scanner.scanTokens();

  if (repl) {
    // 1) Try parse as a single expression.
    Parser exprParser = new Parser(tokens);
    Expr expr = exprParser.parseReplExpression();

    if (!hadError && expr != null) {
      Object value = interpreter.evaluateExpression(expr);
      System.out.println(interpreter.stringifyValue(value));
      return;
    }

    // If it was not a pure expression, fall through and parse statements.
    hadError = false; // reset parse error from the expr attempt, if any
  }

  Parser parser = new Parser(tokens);
  List<Stmt> statements = parser.parse();
  if (hadError) return;

  interpreter.interpret(statements);
}