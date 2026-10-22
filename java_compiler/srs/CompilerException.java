public class CompilerException extends RuntimeException {
    private final String stage;
    private final int line;

    public CompilerException(String stage, int line, String message) {
        super(message);
        this.stage = stage;
        this.line = line;
    }

    public String getStage() {
        return stage;
    }

    public int getLine() {
        return line;
    }
}

