#ifndef CAMERAMARK_LOGOMANAGER_H
#define CAMERAMARK_LOGOMANAGER_H

#include <mutex>
#include <iostream>
#include <memory>

class QPixmap;

namespace CM
{
    enum class CameraIndex : uint8_t
    {
        None = 0,
        ///< empty
        Nikon,
        Sony,
        Canon,
        Apple,
        Panasonic,
        Leica,
        Hasselblad,
        Fujifilm
        /// ...... TODO: need add others
    };

    namespace Tools
    {
        class ResourcesTools;
    }

    class LogoManager
    {
        friend class Tools::ResourcesTools;

    public:
        LogoManager() = default;

        static void Init();

        /**
         * @brief 通过cameraMake参数获取使用的相机
         * @param cameraMake exif Info
         * @return CameraIndex指示相机的制造商
         */
        static CameraIndex resolverCameraIndex(const std::string &cameraMake);

        /**
         * @brief load logo
         * @param cameraIndex 加载的logo的相机
         */
        static void loadCameraLogo(const CameraIndex &cameraIndex);

        /**
         * @brief 获取camera maker logo
         * @param cameraIndex
         * @return logo with QPixmap
         */
        static std::shared_ptr<QPixmap> getCameraMakerLogo(const CameraIndex &cameraIndex);

    protected:
        /**
         * @brief 销毁logo manager 清空所有加载的logo
         */
        static void destroy()
        {
            m_Logos.clear();
            m_CameraMakerMap.clear();
        }

    private:
        static std::unordered_map<CameraIndex, std::shared_ptr<QPixmap>> m_Logos;
        static std::unordered_map<std::string, CameraIndex> m_CameraMakerMap;
        static std::once_flag m_InitFlag;
    };
} // CM

#endif // CAMERAMARK_LOGOMANAGER_H
