//
// Created by Kiril on 17.10.2022.
//

#ifndef DEEPLOM_LINE_H
#define DEEPLOM_LINE_H

#include <iostream>
#include <string>

namespace fileutils
{

    struct Line
    {
        std::string str()
        {
            return _str;
        };

        std::string str() const
        {
            return _str;
        };

        friend std::istream& operator>>(std::istream& is, Line& line)
        {
            return std::getline(is, line._str);
        };

    private:
        std::string _str;
    };

} // fileutils

#endif //DEEPLOM_LINE_H
