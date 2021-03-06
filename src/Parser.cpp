//
//  Parser.cpp
//  zwwC
//
//  Created by Frank  on 2/12/18.
//  Copyright © 2018 Zhongwei Wang. All rights reserved.
//

#include "Parser.hpp"


ExprAST* Parser::parseProgram(){
    std::vector<ExprAST*> stmts;
    while (!isEnd()) {
        std::cout<<getToken()->getvalue()<<std::endl;
        stmts.push_back(parseState());
    }
    ExprAST* result = new ProgramAST(stmts);
    return result;
}

/*
 statement->:
 for(assign;bool;assign)
 if(bool){statements} else{statements}
 while(bool){statements}
 Declare(INT|BOOL )
 */
ExprAST* Parser::parseState(){
    std::cout<<"parsing state"<<std::endl;
    ExprAST* result;
    switch (getToken()->getType()) {
        case T_IF:
            result = parseIF();
            break;
        case T_WHILE:
            result = parseWhile();
            break;
        case T_FOR:
            result = parseFor();
            break;
        case T_DEF:
            result = parseDef();
            break;
        // case T_RETURN:
        //     result = parseReturn();
        //     match(";");
        //     break;
        case T_L_BBRAC:
            result = parseBlock();
            break;
        case T_SET:
            result = parseDeclare();
            match(";");
            break;
        case T_RETURN:
            toNext();//skip the Return
            result = parseBool();
            match(";");
            break;
        case T_ID:
            result = parseAssign();
            match(";");
            break;
        case T_SELFOP:
//            result = new BinopAST(new VariableAST(1), )
            break;
        default:
            result = 0;
            toNext();//skip the error
            break;
    }
    return result;
}

ExprAST* Parser::parseIF(){
    std::cout<<"parsing if"<<std::endl;
    IfAST * result;
    toNext();//skip the first if
    match("(");
    ExprAST * cond = parseBool();
    match(")");
    ExprAST * body = parseState();
    if (getToken()->getType() == T_ELSE) {
        match("else");
        ExprAST * elseb = parseState();
        result = new IfAST(cond,body,elseb);
        return result;
    }
    result = new IfAST(cond,body,NULL);
    return result;
}

ExprAST* Parser::parseWhile(){
    std::cout<<"parsing while"<<std::endl;
    WhileAST * result;
    toNext();//pass the while
    match("(");
    ExprAST * cond = parseBool();
    match(")");
    ExprAST * body = parseState();
    result = new WhileAST(cond,body);
    return  result;
}
ExprAST* Parser::parseFor(){
    std::cout<<"parsing for"<<std::endl;
    toNext();//skip the for
    match("(");
    ExprAST * init = parseDeclare();
    match(";");
    ExprAST * cond = parseBool();
    match(";");
    ExprAST * iter = parseAssign();
    match(")");
    ExprAST * body = parseState();
    ForcallAST* result = new ForcallAST(init,cond,iter,body);
    return result;
}

ExprAST* Parser::parseReturn(){
    ExprAST * result = NULL;
    return result;
}

//Assign-> Array = Bool;
ExprAST* Parser::parseAssign(){
    if(readNext()->getType() == T_L_RBRAC){
        CallfuncAST* res = parseFuncCall();
        return res;
    }
    std::cout<<"parsing assign"<<std::endl;
    VariableAST* var = parseVariable();
    match("=");
    ExprAST* boolexp = parseBool();
    AssignAST *result = new AssignAST(var,boolexp);
   // match(";");
    return result;
}

CallfuncAST* Parser::parseFuncCall(){
    std::string funcName = getToken()->getvalue();
    toNext(); //skip the function name
    match("(");
    std::vector<ExprAST*> p_args;
    while(1){
        p_args.push_back(parseBool());
       // toNext();//skip this arg
        if(getToken()->getType()==T_R_RBRAC) break;
        match(",");
    }
    match(")");
   // std::cout<<getToken()->getvalue()<<std::endl;
    return new CallfuncAST(funcName,p_args);
}


//Declare->Type Array LAssign|.
ExprAST* Parser::parseDeclare(){
    std::cout<<"parsing declare"<<std::endl;
    ExprAST * result;
    toNext(); //skip the 'set' 
    VariableAST * var = parseVariable();
    if (getToken()->getType() == T_EQUAL) {
        match("=");
        ExprAST * assign = parseBool();
        result = new DeclareAST(var,assign);
       // match(";");
        return result;
    }
    result = new DeclareAST(var);
   // match(";");
    return result;
}

//Def func_name(x,y) block
ExprAST* Parser::parseDef(){
    ProtoAST* result = NULL;
    std::vector<std::string> args;
    toNext(); //skip the def
    std::string name = getToken()->getvalue();
    toNext();
    match("(");
    while(1){
        std::string newarg = getToken()->getvalue();
        args.push_back(newarg);
        toNext();//skip this token
        if(getToken()->getvalue() == ")") break;
        match(",");
    }
    match(")");
    result = new ProtoAST(name,args);
    //match("{");
    ExprAST* blo = parseState();
    //match("}");
    ExprAST* res = new FunctionAST(result,blo);
    return res;
}

//implement the block statements
ExprAST* Parser::parseBlock(){
    match("{");
    std::vector<ExprAST*> block;
    while (getToken()->getvalue()!="}") {
        block.push_back(parseState());
    }
    toNext();//skip the }
    ExprAST* result = new BlockAST(block);
    return result;
}

//Array->ID|ID[number]|ID[ID]
ExprAST* Parser::parseArray(){
    ExprAST * result=NULL;
    toNext();//skip the ID
    if (this->getToken()->getType() == T_L_SBRAC) {
        toNext();//skio the [
        if (this->getToken()->getType() == T_ID) {
            result = parseVariable();
        }
        else{
            result = parseState();
        }
    }
    return result;
}



//implement Bool calculation
ExprAST* Parser::parseBool(){
    std::cout<<"parsing calculation"<<std::endl;
    ExprAST * hrel = parseHRel();
    if (getToken()->getType()==T_LLOGICOP) {
        ExprAST * result = parseLBool(hrel);
        return result;
    }
    
    return hrel;
}
ExprAST* Parser::parseLBool(ExprAST*lvalue){
    std::string op = getToken()->getvalue();
    toNext();
    ExprAST * hrel = parseHRel();
    ExprAST * result = new BinopAST(op,lvalue,hrel);
    if (getToken()->getType()==T_LLOGICOP) {
        ExprAST * rvalue = parseLBool(result);
        return rvalue;
    }
    return result;
}
ExprAST* Parser::parseHRel(){
    ExprAST * rel = parseRel();
    if (getToken()->getType()==T_HLOGICOP) {
        ExprAST * result = parseLHRel(rel);
        return result;
    }
    return rel;
}
ExprAST* Parser::parseLHRel(ExprAST*lvalue){
    std::string op = getToken()->getvalue();
    toNext();
    ExprAST * rel = parseRel();
    ExprAST * result = new BinopAST(op,lvalue,rel);
    if (getToken()->getType()==T_HLOGICOP) {
        ExprAST * rvalue = parseLHRel(result);
        return rvalue;
    }
    return result;
}
ExprAST* Parser::parseRel(){
    ExprAST * exp = parseExp();
    if (getToken()->getType()==T_JUDGEOP) {
        ExprAST * result = parseLRel(exp);
        return result;
    }
    return exp;
}
ExprAST* Parser::parseLRel(ExprAST*lvalue){
    std::string op = getToken()->getvalue();
    toNext();
    ExprAST * exp = parseExp();
    ExprAST * result = new BinopAST(op,lvalue,exp);
    if (getToken()->getType()==T_JUDGEOP) {
        ExprAST * rvalue = parseLRel(result);
        return rvalue;
    }
    return result;
}
ExprAST* Parser::parseExp(){
    ExprAST * term = parseTerm();
    if (getToken()->getType()==T_LMATHOP) {
        ExprAST * result = parseLExp(term);
        return result;
    }
    return term;
}

ExprAST* Parser::parseLExp(ExprAST*lvalue){
    std::string op = getToken()->getvalue();
    toNext();
    ExprAST * term = parseTerm();
    ExprAST * result = new BinopAST(op,lvalue,term);
    if (getToken()->getType()==T_LMATHOP) {
        ExprAST * rvalue = parseLExp(result);
        return rvalue;
    }
    return result;
}
ExprAST* Parser::parseTerm(){
    ExprAST * f = parseF();
    if (getToken()->getType()==T_HMATHOP) {
        ExprAST * result = parseLTerm(f);
        return result;
    }
    return f;
}

ExprAST* Parser::parseLTerm(ExprAST*lvalue){
    std::string op = getToken()->getvalue();
    toNext();
    ExprAST * f = parseF();
    ExprAST * result = new BinopAST(op,lvalue,f);
    if (getToken()->getType()==T_HMATHOP) {
        ExprAST * rvalue = parseLTerm(result);
        return rvalue;
    }
    return result;
}

ExprAST* Parser::parseF(){
    ExprAST * result;
    switch (getToken()->getType()) {
        case T_ID:
           result = parseVariable();
            break;
        case T_NUM:
            result= parseNumber();
            break;
        case T_L_RBRAC:
            match("(");
            result = parseBool();
            match(")");
        default:
            result = 0;
            break;
    }
    return result;
}

//implement parse variable, number and array(todo)
VariableAST* Parser::parseVariable(){
    VariableAST* var = new VariableAST(getToken()->getvalue());
    toNext();
    return var;
}

NumberAST* Parser::parseNumber(){
    NumberAST * number = new NumberAST((double)getToken()->getNumvalue());
    toNext();//move to next token
    return number;
}
