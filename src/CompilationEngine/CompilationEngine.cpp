#include "CompilationEngine.hpp"
#include <utility>

CompilationEngine::CompilationEngine(std::string fName) {
    JackTokenizer tokenizer(std::move(fName), "test.tst");
    tokenizer.cleanCode();
    m_code = tokenizer.getAllCodeVector();
    removeTabs(m_code);
    m_currentLine = 0;
    tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));

    while(tempTokens.empty()){
        tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));
        m_currentLine++;
    }
//
    if (tempTokens[0] == "class") {
        compileClass();
    }
}

// make methods work
// make multiple files work together

void CompilationEngine::compileClass() {

    if (isValidName(tempTokens[1])){
        m_currentClassName = tempTokens[1];
        validSubroutineTypes.push_back(m_currentClassName);
    }
    else{
        return;
    }

    m_currentLine++;
    tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));

//    if (JackTokenizer::isValid(validVarDecs, tempTokens[0])) {
        compileClassVarDec();
//    }

    if (tempTokens.back() == "{"){
        insideClass = true;
    }

    compileSubroutine();
    for (auto& v: vmCode){
        std::cout << v << std::endl;
    }
}


void CompilationEngine::compileClassVarDec() {
    std::string currentLine = getNthToken(m_currentLine);
    auto tokens = JackTokenizer::tokenizeCode(currentLine);
    tempTokens = tokens;

    if (tokens[0] == "static" || tokens[0] == "field") {
//        if (JackTokenizer::isValid(validVarTypes, tokens[1])){
            if (isValidName(clearName(tokens[2]))){
                m_currentLine++;

                std::string variables = currentLine.substr(currentLine.find(tempTokens[2]));
                variables = clearName(variables);
                variables.erase(std::remove(variables.begin(), variables.end(), ' '), variables.end());

                CODE variableVector;
                variableVector = splitString(variables, ',');

                int i = 0;
                for (auto &variable: variableVector){
                    std::string name = variableVector[i];

                    if (isValidName(clearName(name))) {
                        auto kind = tempTokens[0];
                        classSymbolTable.insert(clearName(name), tempTokens[1], kind);
                        i++;
                    }
                }
                currentLine = getNthToken(m_currentLine);
                tempTokens = JackTokenizer::tokenizeCode(currentLine);
//                m_currentLine++;
                compileClassVarDec();
            }
//        }

//        std::string token = getNthToken(m_currentLine);
//        m_currentLine++;
//
//        compileClassVarDec();
    }
    else{
        return;
    }
}
std::string removeNonAlphanumeric(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (std::isalnum(c) || c == '_') {
            result += c;
        }
    }
    return result;
}

void CompilationEngine::compileSubroutine() {
    subroutineSymbolTable.display();
    subroutineSymbolTable.reset();
    tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));

    if (tempTokens.empty()){
        return;
    }

    if (JackTokenizer::isValid(validSubroutineDec, tempTokens[0])) {
        if ((JackTokenizer::isValid(validSubroutineTypes, tempTokens[1])) || (tempTokens[1] == m_currentClassName)){
            std::string name = removeNonAlphanumeric(tempTokens[2]);
            if (isValidName(name)){
                if (tempTokens.back() == "{"){
                    insideSubroutine = true;
                }
//                m_currentSubroutineDecType = tempTokens[0];

                m_currentSubroutineDecType = tempTokens[0];
                m_currentSubroutineDef = "function " + m_currentClassName + "." + name ;
                vmCode.push_back(m_currentSubroutineDef);
                m_funcNameIndex = vmCode.size() - 1;

                subroutineTypes[m_currentClassName + "." + name] = tempTokens[1];
                compileParameterList();
                m_currentLine++;
                compileSubroutineBody();
            }
        }

    }
}

std::string CompilationEngine::getNthToken(int n) {
    std::string str;

    if (n >=  m_code.size()){
        return "";
    }

    if (JackTokenizer::isNotEmpty(m_code[n])){
        auto index = m_code[n].find_first_not_of(' ');
        if (index != std::string::npos){
            m_code[n] = m_code[n].substr(index);
            str = m_code[n];
        }
        if (str.empty()){
            m_currentLine++;
            return getNthToken(n+1);
        }
        return str;
    }
    else{
        m_currentLine++;
        return getNthToken(n+1);
    }
}

bool CompilationEngine::isValidName(std::string name) {
    size_t index = name.find_first_not_of(' ');
    size_t end = name.find_last_not_of(' ');

    if ((index != std::string::npos) && (end != std::string::npos)){
        name = name.substr(index, end - index + 1);

    }

    if (JackTokenizer::isValid(validVarTypes, name) || JackTokenizer::isValid(validSubroutineDec, name) ||
        JackTokenizer::isValid(validSymbols, name) || JackTokenizer::isValid(validKeywords, name)) {
        return false;
    }
    else{
        if (std::regex_match(name, std::regex("^(_|[a-zA-Z])[a-zA-Z0-9_]*;?$"))){
            return true;
        }
    }
    return false;
}

void CompilationEngine::compileVarDec() {

    std::string currentLine = getNthToken(m_currentLine);

    currentLine = currentLine.substr(currentLine.find_first_not_of(' '));
    tempTokens = splitString(currentLine, ' ');

    if ((tempTokens[0] == "var")){
//        if (JackTokenizer::isValid(validVarTypes, tempTokens[1])){
            std::string variables = currentLine.substr(currentLine.find(tempTokens[2]));
            variables = clearName(variables);
            variables.erase(std::remove(variables.begin(), variables.end(), ' '), variables.end());

            CODE variableVector;
            variableVector = splitString(variables, ',');

            int i = 0;
            for (auto &variable: variableVector){
                std::string name = variableVector[i];

                if (isValidName(clearName(name))) {
                    auto kind = tempTokens[0];

                    if (kind == "var"){
                        kind = "local";
                    }

                    subroutineSymbolTable.insert(clearName(name), tempTokens[1], kind);
                    i++;
                }
            }
            m_currentLine++;
            std::string token = getNthToken(m_currentLine);
            tempTokens = JackTokenizer::tokenizeCode(token);

            compileVarDec();
        }
//        else{
//            vmCode[m_funcNameIndex] = m_currentSubroutineDef + " " + std::to_string(subroutineSymbolTable.count("local"));
//            return;
//        }
//    }
    else{
        vmCode[m_funcNameIndex] = m_currentSubroutineDef + " " + std::to_string(subroutineSymbolTable.count("local"));
        return;
    }

}

void CompilationEngine::compileParameterList() {
    auto index = std::find(tempTokens.begin(), tempTokens.end(), "(");
    long long paramCount = 0;

    if (index != tempTokens.end() ){
        auto index2 =  std::find(tempTokens.begin(), tempTokens.end(), ")");
        if ((index2 != tempTokens.end()) && (std::distance(tempTokens.begin(), index) + 1 != std::distance(tempTokens.begin(), index2))){
            auto index3 = std::distance(tempTokens.begin(), index);
            if (JackTokenizer::isValid(validVarTypes, tempTokens.at(index3+1))){
                if (isValidName(tempTokens.at(index3+2))){
                    auto tempTokens2 = tempTokens;
                        std::vector<std::string> parameters = removeBrackets(tempTokens2);
                        paramCount = countParameters(parameters);

                        for (long long i = 0; i < (paramCount * 3); i+=3) {
                            subroutineSymbolTable.insert(parameters[i+1], parameters[i], "argument");
                        }
                }
            }
        }
    }
    else{
        return;
    }
}

long long CompilationEngine::countParameters(CODE parameterList) {
    auto count = std::count(parameterList.begin(), parameterList.end(), "int");
    count += std::count(parameterList.begin(), parameterList.end(), "boolean");
    count += std::count(parameterList.begin(), parameterList.end(), "bool");
    count += std::count(parameterList.begin(), parameterList.end(), "char");
    return count;
}

void CompilationEngine::compileSubroutineBody() {
    compileVarDec();

    if (m_currentSubroutineDecType == "constructor"){
       vmCode.push_back("push constant " + std::to_string(classSymbolTable.count("field")));
       vmCode.push_back("call Memory.alloc 1");
       vmCode.push_back("pop pointer 0");
    }
    else if (m_currentSubroutineDecType == "method"){
        subroutineSymbolTable.insert("this", m_currentClassName, "argument");
        vmCode.push_back("push argument 0");
        vmCode.push_back("pop pointer 0");
    }

    while (!(getNthToken(m_currentLine).starts_with("}"))){
        compileStatement("");
    }

    tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));

    if (tempTokens.back() == "}"){
        insideSubroutine = false;
    }

    m_currentLine++;
    compileSubroutine();
}

void CompilationEngine::compileStatement(const std::string& line = "") {
    if (!(line.empty())){
        tempTokens = JackTokenizer::tokenizeCode(line);
        vmCode.push_back("//" + line);
    }
    else{
        tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));
        vmCode.push_back("//" + getNthToken(m_currentLine));
    }
    if (JackTokenizer::isValid(validStatementInitials, tempTokens[0])) {
        if (tempTokens[0] == "do") {
            compileDo();
        }
        else if (tempTokens[0] == "let") {
            compileLet();
        }
        else if (tempTokens[0] == "if"){
            compileIf();
        }
        else if (tempTokens[0] == "return"){
            compileReturn();
        }
        else if (tempTokens[0] == "while"){
            compileWhile();
        }
    }
}

void CompilationEngine::compileDo() {
    compileExpressionList(getNthToken(m_currentLine));
    callSubroutine(getNthToken(m_currentLine));
    vmCode.push_back("pop temp 0");
    m_currentLine++;
}


CODE CompilationEngine::removeBrackets(CODE code) {
    auto start = std::find(code.begin(), code.end(), "(") + 1;
    auto end = std::find(code.begin(), code.end(), ")");

    std::vector<std::string> removed(start, end);
    return removed;
}

std::string CompilationEngine::removeBrackets(const std::string& str, bool inLine) {
    auto start = str.find_first_of('(');
    unsigned long long end;

    if (start == std::string::npos) {
        return str;
    }

    start = start+1;

    if (!(inLine)){
        end = str.find_last_of(')');
    }
    else{
        end = str.find_first_of(')');
    }


    std::string str2(str.begin() + start, str.begin() + end);
    return str2;
}


void CompilationEngine::compileExpression(std::string& expr) {
    CODE exprVec;
    expr = clearName(expr);
    prioritizeBrackets(expr);
    exprVec = getExpressionVector(expr);
    exprVec.erase(std::remove_if(exprVec.begin(), exprVec.end(), [](const std::string& string) {
        return string.empty();
    }), exprVec.end());

    if (exprVec[0] == "-"){
        compileTerm(expr);
    }
    else if (exprVec[0] == "~"){
        expr = removeBrackets(expr, false);
        compileExpression(expr);
        vmCode.push_back("not");
    }
    else{
        compileTerm(exprVec[0]);
    }

    if (exprVec.size() > 1){
        int n = 1;
        while(JackTokenizer::isValid(validOperations, exprVec[n])){
            compileExpression(exprVec[n+1]);

            if (exprVec[n] == "+"){
             vmCode.push_back("add");
            }
            else if (exprVec[n] == "-"){
               vmCode.push_back("sub");
            }
            else if (exprVec[n] == "*"){
                vmCode.push_back("call Math.multiply 2");
            }
            else if (exprVec[n] == "/"){
                vmCode.push_back("call Math.divide 2");
            }
            else if (exprVec[n] == "="){
                vmCode.push_back("eq");
            }
            else if (exprVec[n] == ">"){
                vmCode.push_back("gt");
            }
            else if (exprVec[n] == "<"){
                vmCode.push_back("lt");
            }
            else if (exprVec[n] == "&"){
                vmCode.push_back("and");
            }
         n += 2;
      }
   }
}

bool CompilationEngine::isNumber(std::string &str) {
    return !str.empty() && str.find_first_not_of("0123456789") == std::string::npos;
}

bool CompilationEngine::isNumber(char &ch) {
    if (JackTokenizer::isValid(validPositiveInt, ch)){
        return true;
    }
    return false;
}

bool CompilationEngine::isCharacterPresent(const std::string &str1, char c) {
    for (auto x: str1){
        if (c == x){
            return true;
        }
    }
    return false;
}

void CompilationEngine::compileExpressionList(std::string expressions) {
    std::string exprList = removeBrackets(expressions, false);

    if (exprList.empty()) {
        return;
    }

    exprList = prioritizeBrackets(exprList);
    CODE exprVec;

    for (char ch : exprList) {
        std::string temp = {ch};
        exprVec.push_back(temp);
    }

    std::vector<int> sepIndex;
    int k = 0;

    for (auto it = exprVec.begin(); it != exprVec.end(); ++it) {
        if (*it == ",") {
            if (std::next(it, 2) != exprVec.end() && (*(std::next(it, 2)) == "," || *(std::next(it, 2)) == ")")) {
                continue;
            }
            sepIndex.push_back(k);
        }
        k++;
    }

    sepIndex.push_back(-1);

    int start = 0;
    std::string expression;

    for (size_t j = 0; j < sepIndex.size(); j++) {
        int index = sepIndex[j];

        if (index >= 0) {
            std::vector<std::string>::iterator startIt = exprVec.begin() + start;
            std::vector<std::string>::iterator endIt = exprVec.begin() + index;

            try {
                expression.clear();
                for (auto it = startIt; it != endIt; ++it) {
                    expression += *it;
                }
            } catch (const std::exception& e) {
                std::cout << "error" << std::endl;
                break;
            }
        } else {
            if (index != -1) {
                break;
            }

            auto startIt = exprVec.begin() + start;
            expression.clear();

            for (auto it = startIt; it != exprVec.end(); ++it) {
                expression += *it;
            }
        }

        compileExpression(expression);
        start = index + 1;
    }
}


void CompilationEngine::compileTerm(std::string term) {
    term = clearName(term);

    if (term.starts_with('(')){
        term.erase(0, 1);
    }
    else if (term.ends_with(')')){
        term.pop_back();
    }
    if (isNumber(term)){
        vmCode.push_back("push constant " + term);
    }
    else if ((subroutineSymbolTable.index(clearName(term).c_str()) != -1) || (classSymbolTable.index(clearName(term).c_str()) != -1)){
        if (subroutineSymbolTable.index(clearName(term).c_str()) != -1){
            vmCode.push_back("push " + subroutineSymbolTable.kind(term) + " " + std::to_string(subroutineSymbolTable.index(term.c_str())));
        }
        else if (classSymbolTable.index(term.c_str()) != -1){
            vmCode.push_back("push " + classSymbolTable.kind(term) + " " + std::to_string(classSymbolTable.index(term.c_str())));
        }
    }
    else if ((term.starts_with('(')) && (term.ends_with(')'))){
        compileExpression(term);
    }
    else if (term[0] == '-'){
        vmCode.push_back("push constant " + term.substr(1));
        vmCode.push_back("neg");
    }
    else if (reservedValues.find(term) != reservedValues.end()){
        if (reservedValues[term] == "-1"){
            vmCode.push_back("push constant 1");
            vmCode.push_back("neg");
        }
        else{
            vmCode.push_back("push constant " + reservedValues[term]);
        }
    }
}

CODE CompilationEngine::getExpressionVector(std::string expr) {
    CODE exprVec;

    std::vector<int> sepIndex;
    int k = 0;

    if (expr.length() == 1){
        exprVec.push_back(expr);
    }

    expr.erase(std::remove(expr.begin(), expr.end(), ' '), expr.end());

    for (char expression: expr){
        if ((expression == '+') || (expression == '-') || (expression == '*') || (expression == '/') || (expression == '&') || (expression == '|') || (expression == '<') || (expression == '>') || (expression == '=') || (expression == '~')){
            sepIndex.push_back(k);
        }
        k++;
    }

    sepIndex.push_back(-1);

    if (sepIndex.size() == 2){
        exprVec.push_back(expr.substr(0, sepIndex[0]));
        exprVec.push_back(expr.substr(sepIndex[0],1));
        exprVec.push_back(expr.substr(sepIndex[0]+1));
        return exprVec;
    }

    int start = 0;

    std::string expression;
    std::string op;

    for (int index : sepIndex){
        if (index != -1){

            expression = expr.substr(start, index - start);
            op = expr.substr(index,1);
        }
        else{
            expression = expr.substr(start, expr.length());
        }
        if (!(expression.empty())){
            exprVec.push_back(clearName(expression));
            if (!(op.empty())){
                exprVec.push_back(clearName(op));
            }
            else{
                break;
            }
        }
        else if (op == "~"){
            exprVec.push_back(op);
        }


        op = "";
        start = index + 1;
    }
    return exprVec;
}

std::string CompilationEngine::clearName(std::string name) {
    size_t nameStart = name.find_first_not_of(' ');
    size_t spaceEnd = name.find_last_not_of(' ');

    if (nameStart != std::string::npos && spaceEnd != std::string::npos) {
        name =  name.substr(nameStart, spaceEnd - nameStart + 1);

        if (name.ends_with(";")) {
            name.pop_back();
        }
    }

    return name;
}

std::string CompilationEngine::prioritizeBrackets(std::string &expression) {
    CODE expressionVec;
    std::string temp;

    expressionVec = splitString(expression, ',');

    if (std::find(expressionVec.begin(), expressionVec.end(), ")") == expressionVec.end()){
        return expression;
    }

    for (auto &expr: expressionVec) {
        if ((expr.find('(') != std::string::npos) && (expr.find(')') != std::string::npos)) {
            expr.erase(std::remove_if(expr.begin(), expr.end(), ::isspace), expr.end());
            auto idx1 = expr.find_first_of('(');
            auto idx2 = expr.find_first_of(')');

            std::string transformed;
            std::string op;
            std::string oldStart;


            transformed = expr.substr(idx1 + 1, (idx2 - idx1) - 1);
            op = expr.substr(idx1 - 1, 1);
            oldStart = expr.substr(0, idx1 - 1);

            if (op == "-") {
                transformed = op + transformed;
            } else {
                transformed.append(op);
            }

            if (oldStart[0] == '-') {
                if (op == "+") {
                    oldStart[0] = '-';
                } else if (op == "-") {
                    oldStart[0] = '+';
                } else {
                    oldStart = "(" + oldStart + ")";
                }
            }

            transformed.append(oldStart);

            while (transformed.find('(') != std::string::npos) {
                transformed = prioritizeBrackets(transformed);
            }

            if (expr != expressionVec.back()) {
                temp.append(transformed + ",");
            } else {
                temp.append(transformed);
            }
        }
    }
    return temp;
}

void CompilationEngine::callSubroutine(std::string line) {
    CODE lineVec = splitString(line, ' ');
    std::string funcName;
    int objAddition = 0;

    if (lineVec[0] == "do"){
        std::string funcName2 = lineVec[1];
        funcName2 = lineVec[1].substr(0,  funcName2 .find('('));
        if (isCharacterPresent(funcName2 , '.')){
            std::string objName = funcName2 .substr(0, funcName2 .find('.'));
            funcName2 = funcName2.substr(funcName2 .find('.') + 1);
            if (JackTokenizer::isValid(validVarTypes, objName)){
                std::cerr << "Cannot use '.' operator on predefined types." << std::endl;
            }
            else if ((subroutineSymbolTable.index(objName.c_str()) != -1) || (classSymbolTable.index(objName.c_str())) != -1) {
                if ((!(JackTokenizer::isValid(validVarTypes, objName)))) {
                    if (subroutineSymbolTable.index(objName.c_str()) != -1) {
                        vmCode.push_back("push " + subroutineSymbolTable.kind(objName) + " " +
                                         std::to_string(subroutineSymbolTable.index(objName.c_str())));
                        funcName2 = subroutineSymbolTable.type(objName) + "." + funcName2;
                        funcName = clearName(funcName2);
                        objAddition = 1;
                    }
                    else {
                        vmCode.push_back("push " + classSymbolTable.kind(objName) + " " +
                        std::to_string(classSymbolTable.index(objName.c_str())));
                        funcName2 = classSymbolTable.type(objName) + "." + funcName2;
                        funcName = clearName(funcName2);
                        objAddition = 1;
                    }
                }
            }
            else{
                funcName = lineVec[1].substr(0, lineVec[1].find('('));
                funcName = clearName(funcName);
            }
        }
    }
    else{
        funcName = lineVec[0].substr(0, lineVec[0].find('('));
    }
    std::string params;
    params = removeBrackets(line, false);
    auto paramsVec = splitString(params, ',');

    vmCode.push_back("call " + funcName + " " + std::to_string(paramsVec.size() + objAddition));
}

void CompilationEngine::compileLet() {
    std::string currentLine = getNthToken(m_currentLine);
    tempTokens = splitString(currentLine, ' ');
    tempTokens.erase(std::remove(tempTokens.begin(), tempTokens.end(), " "), tempTokens.end());
    tempTokens.erase(std::remove(tempTokens.begin(), tempTokens.end(), ""), tempTokens.end());

    std::string varName = tempTokens[1];

    std::string value;
    value = currentLine.substr(currentLine.find(tempTokens[3]));
    value.erase(0, value.find_first_not_of(' '));
    value.erase(value.find_last_not_of(' ') + 1);
    std::regex callPattern("^[a-zA-Z_][a-zA-Z0-9_]{0,}[.][a-zA-Z_][a-zA-Z0-9]*\\(");

    if (std::regex_search(value, callPattern)){
        if (value.ends_with(");")){
            compileExpressionList(getNthToken(m_currentLine));
            std::string funcName = value.substr(0, value.find(';'));
            callSubroutine(funcName);
        }
    }
    else{
        CODE expressions = splitString(currentLine, '=');
        std::string expression = expressions[1];
        expression = clearName(expression);
        compileExpressionList(expression);
    }

    if (m_currentSubroutineDecType == "constructor"){
        vmCode.push_back("pop this " + std::to_string(classSymbolTable.index(varName.c_str())));
    }
    else{
        vmCode.push_back("pop " + subroutineSymbolTable.kind(varName) + " " + std::to_string(subroutineSymbolTable.index(varName.c_str())));
    }

    m_currentLine++;
}

void CompilationEngine::compileReturn() {
    std::string currentLine = getNthToken(m_currentLine);
    tempTokens = splitString(currentLine, ' ');
    m_currentLine++;

//  All constructors HAVE TO return this, even if they don't do anything with it.
    if (m_currentSubroutineDecType == "constructor"){
        vmCode.push_back("push pointer 0");
        vmCode.push_back("return");
    }
    else if ((tempTokens.size() == 1) || tempTokens[1] == ";"){
        vmCode.push_back("push constant 0");
        vmCode.push_back("return");
    }
    else{
        std::string expression;

        expression = currentLine.substr(7);
        expression = clearName(expression);
        compileExpression(expression);
        vmCode.push_back("return");

        if (tempTokens.back() == "}"){
            insideSubroutine = false;
        }
    }
}


std::string& ltrim(std::string& str) {
    auto it = std::find_if(str.begin(), str.end(), [](char ch) {
        return !std::isspace(ch);
    });
    str.erase(str.begin(), it);
    return str;
}

std::string& rtrim(std::string& str) {
    auto it = std::find_if(str.rbegin(), str.rend(), [](char ch) {
        return !std::isspace(ch);
    });
    str.erase(it.base(), str.end());
    return str;
}

std::string& trim(std::string& str) {
    return ltrim(rtrim(str));
}

std::vector<std::string> CompilationEngine::splitString(std::string& str, char delim) {
    std::vector<std::string> splitVec;
    std::string split;

    std::size_t start = 0;
    std::size_t end = 0;

    while ((end = str.find(delim, start)) != std::string::npos) {
        split = str.substr(start, end - start);
        trim(split);
        if (!split.empty())
            splitVec.push_back(split);
        start = end + 1;
    }

    split = str.substr(start);
    trim(split);
    if (!split.empty())
        splitVec.push_back(split);

    return splitVec;
}

void CompilationEngine::compileIf() {
    std::string currentLine = getNthToken(m_currentLine);
    tempTokens = splitString(currentLine, ' ');
    std::string expression;
    bool insideIf = false;
    bool inlineIf = false;

    if (currentLine.back() == '{'){
        insideIf = true;
        expression = removeBrackets(currentLine, false);
    }
    else if (currentLine.back() == '}'){
        expression = removeBrackets(currentLine, true);
        inlineIf = true;
    }

    compileExpression(expression);
    vmCode.push_back("not");

    if (inlineIf){
        currentLine = currentLine.substr(currentLine.find('{')+1, currentLine.find('}') - currentLine.find('{')-1);
        currentLine = clearName(currentLine);
        compileStatement(currentLine);
        return;
    }

    ++m_ifLabelCount;
    vmCode.push_back("if-goto " + ELSE_LABEL_PREFIX + std::to_string(m_ifLabelCount));

//    if ()

    while((std::find(tempTokens.begin(), tempTokens.end(), "else{") == tempTokens.end()) || (std::find(tempTokens.begin(), tempTokens.end(), "else") == tempTokens.end())){
        m_currentLine++;
        if ((std::find(tempTokens.begin(), tempTokens.end(), "else{") != tempTokens.end()) || (std::find(tempTokens.begin(), tempTokens.end(), "else") != tempTokens.end())){
            break;
        }
//        else{
//            std::string initial = getNthToken(m_currentLine);
//            auto k = JackTokenizer::tokenizeCode(initial);
//            if (std::find(validStatementInitials.begin(), validStatementInitials.end(), k[0]) != validStatementInitials.end()){
//
//            }
//            break;
//        }
        compileStatement();
        tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));
    }

    vmCode.push_back("goto " + CONTINUE_IF_LABEL_PREFIX + std::to_string(m_continueIfLabelCount));

    if (insideIf){
        insideIf = false;

        if (getNthToken(m_currentLine) == "}"){
            m_currentLine++;
        }
        if (getNthToken(m_currentLine).find("else") != std::string::npos){
            m_currentLine++;
        }

        vmCode.push_back("label " + ELSE_LABEL_PREFIX + std::to_string(m_ifLabelCount));
        --m_ifLabelCount;

        currentLine = getNthToken(m_currentLine);
        tempTokens = splitString(currentLine, ' ');
        while(std::find(tempTokens.begin(), tempTokens.end(), "}") == tempTokens.end()){
            compileStatement();
        }
        m_currentLine++;
        vmCode.push_back("label " + CONTINUE_IF_LABEL_PREFIX + std::to_string(m_continueIfLabelCount));
        m_continueIfLabelCount++;
    }
}

void CompilationEngine::compileWhile() {
    std::string currentLine = getNthToken(m_currentLine);
    std::string expression = removeBrackets(currentLine, false);
    int contCount = m_continueIfLabelCount;

    vmCode.push_back("label " + WHILE_LABEL_PREFIX + std::to_string(m_whileLabelCount));
    compileExpression(expression);
    vmCode.push_back("not");

    vmCode.push_back("if-goto " + CONTINUE_WHILE_LABEL_PREFIX + std::to_string(m_continueWhileLabelCount));

    m_currentLine++;
    while(std::find(tempTokens.begin(), tempTokens.end(), "}") == tempTokens.end()){
        compileStatement();
        tempTokens = JackTokenizer::tokenizeCode(getNthToken(m_currentLine));
    }

    vmCode.push_back("goto " + WHILE_LABEL_PREFIX + std::to_string(m_whileLabelCount));
    m_whileLabelCount++;

    vmCode.push_back("label " + CONTINUE_WHILE_LABEL_PREFIX + std::to_string(m_continueWhileLabelCount));
    m_continueWhileLabelCount++;
    m_currentLine++;
}

void CompilationEngine::removeTabs(std::vector<std::string>& string_vector) {
    for (int i = 0; i < string_vector.size(); i++) {
        string_vector[i].erase(std::remove(string_vector[i].begin(), string_vector[i].end(), '\t'), string_vector[i].end());
        if (string_vector[i].empty()) {
            string_vector.erase(string_vector.begin() + i);
            i--;
        }
    }
}

