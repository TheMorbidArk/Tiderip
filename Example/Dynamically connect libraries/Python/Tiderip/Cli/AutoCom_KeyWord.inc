if (buf[0] == 'T') {
    linenoiseAddCompletion(lc, "Tide");
} else if (buf[0] == 'b') {
    linenoiseAddCompletion(lc, "break");
} else if (buf[0] == 'c') {
    linenoiseAddCompletion(lc, "class");
    linenoiseAddCompletion(lc, "continue");
} else if (buf[0] == 'e') {
    linenoiseAddCompletion(lc, "elif");
    linenoiseAddCompletion(lc, "else");
} else if (buf[0] == 'f') {
    linenoiseAddCompletion(lc, "false");
    linenoiseAddCompletion(lc, "for");
    linenoiseAddCompletion(lc, "fun");
} else if (buf[0] == 'i') {
    linenoiseAddCompletion(lc, "if");
    linenoiseAddCompletion(lc, "import");
    linenoiseAddCompletion(lc, "is");
} else if (buf[0] == 'n') {
    linenoiseAddCompletion(lc, "null");
} else if (buf[0] == 'r') {
    linenoiseAddCompletion(lc, "return");
} else if (buf[0] == 's') {
    linenoiseAddCompletion(lc, "static");
    linenoiseAddCompletion(lc, "super");
} else if (buf[0] == 't') {
    linenoiseAddCompletion(lc, "this");
    linenoiseAddCompletion(lc, "true");
} else if (buf[0] == 'w') {
    linenoiseAddCompletion(lc, "while");
}
