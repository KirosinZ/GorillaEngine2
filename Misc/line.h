//
// Created by Kiril on 17.10.2022.
//

#ifndef DEEPLOM_LINE_H
#define DEEPLOM_LINE_H

#include <iostream>
#include <string>

namespace fileutils
{

    class Line : std::string
    {
        friend std::istream& operator>>(std::istream& is, Line& line);
    };

} // fileutils

#endif //DEEPLOM_LINE_H
