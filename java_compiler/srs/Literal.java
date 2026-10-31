public class Literal extends Expression {
    private final String value;
    private final String type;

    public Literal(String value, String type, int line) {
        super(line);
        this.value = value;
        this.type = type;
    }

    public String getValue() {
        return value;
    }

    public String getType() {
        return type;
    }

    @Override
    public String print() {
        if (type.equals("char")) {
            return "'" + value + "'";
        }
        return value;
    }
}
