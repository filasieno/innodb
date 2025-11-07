fn test_expressions(a: int, b: int, c: int): int {
    var result = a + b * c;
    if (result > 10 && result < 100) {
        result = result << 1;
    } else {
        result = result >> 1;
    }
    return result;
}
