//
//  main.cpp
//  zwwC
//
//  Created by Frank  on 1/25/18.
//  Copyright © 2018 Zhongwei Wang. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "Scanner.h"
#include "Parser.hpp"

int main(int argc, const char * argv[]) {
    // insert code here...
    std::ifstream infile("/Users/wang/Desktop/homework/zwwC/test.zc");
    Scanner * scn = new Scanner();
    scn->scan(infile);
    scn->printToken();
    Parser * parser = new Parser(scn->getList());
    parser->parseProgram();
    return 0;
}
