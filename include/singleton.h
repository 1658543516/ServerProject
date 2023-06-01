//
// Created by mrpiao on 23-6-1.
//
#include <iostream>
#include <memory>

#ifndef SERVERPROJECT_SINGLETON_H
#define SERVERPROJECT_SINGLETON_H
namespace srvpro {

    template<class T, class X = void, int N = 0>
    class Singleton {
    public:
        static T* GetInstance() {
            static T v;
            return &v;
        }
    };

    template<class T, class X = void, int N = 0>
    class SingletonPtr {
        static std::shared_ptr<T> GetInstance() {
            static std::shared_ptr<T> v(new T);
            return v;
        }
    };

}
#endif //SERVERPROJECT_SINGLETON_H
