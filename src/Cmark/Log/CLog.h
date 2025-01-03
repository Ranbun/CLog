#ifndef CAMERAMARK_CLOG_H
#define CAMERAMARK_CLOG_H

#include <QDebug>
#include <mutex>

namespace CM
{
    class CLog
    {
    public:
        static void Init();

        template <typename T>
        static void PrintMes(const T &value)
        {
            if (auto typeName = QMetaType::typeName(qMetaTypeId<T>()))
            {
#if _DEBUG
                qDebug() << value;
#endif
            }
        }
        template <typename T>
        static void Info(const T &value)
        {
            if (auto typeName = QMetaType::typeName(qMetaTypeId<T>()))
            {
#if _DEBUG
                qInfo() << value;
#endif
            }
        }

        template <typename T>
        static void Warning(const T &value)
        {
            if (auto typeName = QMetaType::typeName(qMetaTypeId<T>()))
            {
#if _DEBUG
                qWarning() << value;
#endif
            }
        }

        template <typename T>
        static void Fatal(const T &value)
        {
            Q_ASSERT(false);
        }

        template <>
        static void Fatal(const std::string &value)
        {

#if _DEBUG
            qFatal("%s", value.c_str());
#endif
        }

    private:
        static std::once_flag m_InitFlag;
    };

    inline CLog g_Log;
#define CLogInstance CM::g_Log

} // CM

#endif // CAMERAMARK_CLOG_H
