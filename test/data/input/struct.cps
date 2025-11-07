fn calculate_distance(x1: int, y1: int, x2: int, y2: int): int {
    var dx = x1 - x2;
    var dy = y1 - y2;
    return dx * dx + dy * dy;
}
