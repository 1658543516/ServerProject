//
// Created by mrpiao on 23-7-6.
//

#ifndef SERVERPROJECT_FD_MANAGER_H
#define SERVERPROJECT_FD_MANAGER_H

#include <memory>
#include "thread.h"
#include "iomanager.h"

namespace srvpro {

    class FdCtx : public std::enable_shared_from_this<FdCtx> {
    public:
        typedef std::shared_ptr<FdCtx> ptr;

        FdCtx(int fd);

        ~FdCtx();

        bool isSocket() const { return m_isSocket; }

        bool isClose() const { return m_isClosed; }

        void setUserNonblock(bool v) { m_userNonblock = v; }

        bool getUserNonblock() const { return m_userNonblock; }

        void setSysNonblock(bool v) { m_sysNonblock = v; }

        bool getSysNonblock() const { return m_sysNonblock; }

        void setTimeout(int type, uint64_t v);

        uint64_t getTimeout(int type);

    private:
        bool init();

    private:
        bool m_isInit: 1;
        bool m_isSocket: 1;
        bool m_sysNonblock: 1;
        bool m_userNonblock: 1;
        bool m_isClosed: 1;
        int m_fd;

        uint64_t m_recvTimeout;
        uint64_t m_sendTimeout;

        srvpro::IOManager *m_iomanager;
    };

    class FdManager {
    public:
        typedef RWMutex RWMutexType;

        /**
         * @brief 无参构造函数
         */
        FdManager();

        /**
         * @brief 获取/创建文件句柄类FdCtx
         * @param[in] fd 文件句柄
         * @param[in] auto_create 是否自动创建
         * @return 返回对应文件句柄类FdCtx::ptr
         */
        FdCtx::ptr get(int fd, bool auto_create = false);

        /**
         * @brief 删除文件句柄类
         * @param[in] fd 文件句柄
         */
        void del(int fd);

    private:
        /// 读写锁
        RWMutexType m_mutex;
        /// 文件句柄集合
        std::vector<FdCtx::ptr> m_datas;
    };

}


#endif //SERVERPROJECT_FD_MANAGER_H
