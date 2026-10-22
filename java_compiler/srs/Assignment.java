public class Assignment extends Statement {
    private final String name;
    private final Expression value;

    public Assignment(String name, Expression value, int line) {
        super(line);
        this.name = name;
        this.value = value;
    }

    public String getName() {
        return name;
    }

    public Expression getValue() {
        return value;
    }

    @Override
    public String print() {
        return "(assign " + name + " " + value.print() + ")";
    }
}
