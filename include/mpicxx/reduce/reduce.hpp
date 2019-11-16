#pragma once
#ifndef REDUCE_HPP_
#define REDUCE_HPP_

#include <string>


class Reduce
{
public:

    Reduce() : hello_{ "Hello" }, world_{ "World" } { }

    const std::string& hello() const { return hello_; }

    const std::string& world() const { return world_; }

    int generateRandomNumber() const { return 4'000; }

     int headerFunction(int number)
     {
         if (number % 2 == 0)
         {
             return number / 2;
         }
         return 3;
     }
private:
    std::string hello_;
    std::string world_;
    
};

#endif // !REDUCE_HPP_
