import java.util.List;

public class Compiler {
    private List<Token> tokens;
    private Program program;
    private SemanticAnalyzer semanticAnalyzer;
    private List<TACInstruction> tac;
    private List<String> targetCode;

    public void compile(String source) {
        // 1. Lexical Analysis
        Lexer lexer = new Lexer(source);
        tokens = lexer.tokenize();

        // 2. Syntax Analysis + AST
        Parser parser = new Parser(tokens);
        program = parser.parse();

        // 3. Semantic Analysis
        semanticAnalyzer = new SemanticAnalyzer();
        semanticAnalyzer.analyze(program);

        // 4. Intermediate Code Generation
        TACGenerator tacGenerator = new TACGenerator();
        tac = tacGenerator.generate(program);

        // 5. Target Code Generation
        CodeGenerator codeGenerator = new CodeGenerator();
        targetCode = codeGenerator.generate(tac);
    }

    public List<Token> getTokens() {
        return tokens;
    }

    public Program getProgram() {
        return program;
    }

    public SemanticAnalyzer getSemanticAnalyzer() {
        return semanticAnalyzer;
    }

    public List<TACInstruction> getTAC() {
        return tac;
    }

    public List<String> getTargetCode() {
        return targetCode;
    }
}

