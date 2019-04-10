#include "cameraCaptureTexture.h"

CameraCaptureTexture::CameraCaptureTexture()
: sp::OpenGLTexture(sp::Texture::Type::Dynamic, "CameraCaptureTexture")
{
    capture = nullptr;
}

bool CameraCaptureTexture::open(int camera_index)
{
    close();
    
    capture = new sp::io::CameraCapture(camera_index);
    if (!capture->isOpen())
    {
        close();
        return false;
    }
    return true;
}

void CameraCaptureTexture::close()
{
    if (capture)
        delete capture;
    capture = nullptr;
}

void CameraCaptureTexture::bind()
{
    if (capture)
    {
        sp::Image image(capture->getFrame());
        if (image.getSize().x > 0)
            setImage(std::move(image));
    }
    sp::OpenGLTexture::bind();
}

sp::Image CameraCaptureTexture::getFrame()
{
    if (capture)
    {
        for(int n=0; n<20; n++)
        {
            sp::Image image(capture->getFrame());
            if (image.getSize().x > 0)
                return image;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    return sp::Image();
}
