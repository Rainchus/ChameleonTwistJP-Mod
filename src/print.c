#include "../include/ct1.h"

// Wrapper for in game printText() until we add a custom function
void textPrint(f32 xPos, f32 yPos, f32 scale, void *text, s32 num) {
    printText(xPos, yPos, 0, scale, 0, 0, text, num);
}

enum textTable {    // Text table for EUC-JP (Preceeded with 0xA3)
    space = 0xBA,
    zero = 0xB0,
    one = 0xB1,
    two = 0xB2,
    three = 0xB3,
    four = 0xB4,
    five = 0xB5,
    six = 0xB6,
    seven = 0xB7,
    eight = 0xB8,
    nine = 0xB9,
    A = 0xC1,
    B = 0xC2,
    C = 0xC3,
    D = 0xC4,
    E = 0xC5,
    F = 0xC6,
    G = 0xC7,
    H = 0xC8,
    I = 0xC9,
    J = 0xCA,
    K = 0xCB,
    L = 0xCC,
    M = 0xCD,
    N = 0xCE,
    O = 0xCF,
    P = 0xD0,
    Q = 0xD1,
    R = 0xD2,
    S = 0xD3,
    T = 0xD4,
    U = 0xD5,
    V = 0xD6,
    W = 0xD7,
    X = 0xD8,
    Y = 0xD9,
    Z = 0xDA,
    a = 0xE1,
    b = 0xE2,
    c = 0xE3,
    d = 0xE4,
    e = 0xE5,
    f = 0xE6,
    g = 0xE7,
    h = 0xE8,
    i = 0xE9,
    j = 0xEA,
    k = 0xEB,
    l = 0xEC,
    m = 0xED,
    n = 0xEE,
    o = 0xEF,
    p = 0xF0,
    q = 0xF1,
    r = 0xF2,
    s = 0xF3,
    t = 0xF4,
    u = 0xF5,
    v = 0xF6,
    w = 0xF7,
    x = 0xF8,
    y = 0xF9,
    z = 0xFA,
};