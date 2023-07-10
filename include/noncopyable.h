//
// Created by mrpiao on 23-7-10.
//

#ifndef SERVERPROJECT_NONCOPYABLE_H
#define SERVERPROJECT_NONCOPYABLE_H

namespace srvpro {
    class Noncopyable {
    public:
        Noncopyable() = default;
        ~Noncopyable() = default;
        Noncopyable(const Noncopyable&) = delete;
        Noncopyable operator=(const Noncopyable&&) = delete;

    };
}

#endif //SERVERPROJECT_NONCOPYABLE_H
