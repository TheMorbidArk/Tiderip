if (!strcasecmp(buf, "Tide")){
    *color = 35;
    *bold = 0;
    return " <Name> = <Value>";
} else if (!strcasecmp(buf, "if")) {
    *color = 35;
    *bold = 0;
    return " (Expression) {Statement}";
} else if (!strcasecmp(buf, "elif")) {
    *color = 35;
    *bold = 0;
    return " (Expression){Statement}";
} else if (!strcasecmp(buf, "else")) {
    *color = 35;
    *bold = 0;
    return " {Statement}";
} else if (!strcasecmp(buf, "for")) {
    *color = 35;
    *bold = 0;
    return " <IndexName> (ValueName) {Statement}";
} else if (!strcasecmp(buf, "fun")) {
    *color = 35;
    *bold = 0;
    return " <FunName> (Arguments) {Statement}";
} else if (!strcasecmp(buf, "import")) {
    *color = 35;
    *bold = 0;
    return " <ImportName>";
} else if (!strcasecmp(buf, "class")) {
    *color = 35;
    *bold = 0;
    return " <ClassName> {Statement}";
}
