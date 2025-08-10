#include <cssom/om.hpp>

#include <iostream>

int main(int argc, const char** argv){
    cssom::sheet css;

    std::cout << "num styles = " << css.styles.size() << std::endl;

    return 0;
}
