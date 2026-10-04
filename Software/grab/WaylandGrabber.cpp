/*
 * WaylandGrabber.cpp
 *
 *  Created on: 04.10.2026
 *      Project: Lightpack
 */

#include "WaylandGrabber.hpp"

#ifdef WAYLAND_QT_GRAB_SUPPORT

#include <QGuiApplication>
#include <QMediaCaptureSession>
#include <QScreen>
#include <QScreenCapture>
#include <QVideoFrame>
#include <QVideoSink>
#include <memory>

struct WaylandGrabberData
{
	std::unique_ptr<QScreenCapture> capture;
	std::unique_ptr<QMediaCaptureSession> session;
	std::unique_ptr<QVideoSink> sink;
	QByteArray frameBytes;
	int bytesPerLine{0};
	BufferFormat format{BufferFormatArgb};
	bool hasFrame{false};
};

WaylandGrabber::WaylandGrabber(QObject *parent, GrabberContext *context)
	: GrabberBase(parent, context)
{
}

WaylandGrabber::~WaylandGrabber()
{
	freeScreens();
}

void WaylandGrabber::startGrabbing()
{
	GrabberBase::startGrabbing();
	for (const GrabbedScreen& screen : _screensWithWidgets) {
		if (WaylandGrabberData* d = reinterpret_cast<WaylandGrabberData*>(screen.associatedData)) {
			d->capture->setActive(true);
		}
	}
}

void WaylandGrabber::stopGrabbing()
{
	for (const GrabbedScreen& screen : _screensWithWidgets) {
		if (WaylandGrabberData* d = reinterpret_cast<WaylandGrabberData*>(screen.associatedData)) {
			d->capture->setActive(false);
		}
	}
	GrabberBase::stopGrabbing();
}

QList<ScreenInfo> *WaylandGrabber::screensWithWidgets(QList<ScreenInfo> *result, const QList<GrabWidget *> &grabWidgets)
{
	result->clear();

	const QList<QScreen*> allScreens = QGuiApplication::screens();
	for (QScreen* screen : allScreens) {
		const QRect screenRect = screen->geometry();
		for (GrabWidget* widget : grabWidgets) {
			if (screenRect.intersects(widget->deviceFrameGeometry())) {
				ScreenInfo info;
				info.handle = screen;
				info.rect = screenRect;
				result->append(info);
				break;
			}
		}
	}
	return result;
}

bool WaylandGrabber::reallocate(const QList<ScreenInfo> &screens)
{
	freeScreens();

	for (const ScreenInfo& screenInfo : screens) {
		QScreen* screen = reinterpret_cast<QScreen*>(screenInfo.handle);
		if (screen == nullptr) {
			qWarning() << Q_FUNC_INFO << "null screen handle";
			continue;
		}

		WaylandGrabberData* d = new WaylandGrabberData();
		d->capture = std::make_unique<QScreenCapture>();
		d->session = std::make_unique<QMediaCaptureSession>();
		d->sink = std::make_unique<QVideoSink>();

		d->capture->setScreen(screen);
		d->session->setScreenCapture(d->capture.get());
		d->session->setVideoSink(d->sink.get());

		connect(d->capture.get(), &QScreenCapture::errorOccurred, this,
			[screen](QScreenCapture::Error error, const QString &errorString) {
				qWarning() << "Wayland screen capture error on" << screen->name() << ":" << error << errorString;
			});

		connect(d->sink.get(), &QVideoSink::videoFrameChanged, this,
			[d](const QVideoFrame &frame) {
				if (!frame.isValid())
					return;
				QImage image = frame.toImage();
				if (image.isNull())
					return;
				switch (image.format()) {
				case QImage::Format_ARGB32:
				case QImage::Format_ARGB32_Premultiplied:
				case QImage::Format_RGB32:
					d->format = BufferFormatArgb;
					break;
				case QImage::Format_RGBA8888:
				case QImage::Format_RGBA8888_Premultiplied:
				case QImage::Format_RGBX8888:
					d->format = BufferFormatRgba;
					break;
				case QImage::Format_BGRA8888:
				case QImage::Format_BGRA8888_Premultiplied:
				case QImage::Format_BGRX8888:
					d->format = BufferFormatBgra;
					break;
				default:
					image = image.convertToFormat(QImage::Format_ARGB32);
					d->format = BufferFormatArgb;
					break;
				}
				d->bytesPerLine = image.bytesPerLine();
				d->frameBytes = QByteArray(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes());
				d->hasFrame = !d->frameBytes.isEmpty();
			});

		if (isGrabbingStarted()) {
			d->capture->setActive(true);
		}

		GrabbedScreen grabbedScreen;
		grabbedScreen.associatedData = d;
		grabbedScreen.screenInfo = screenInfo;
		grabbedScreen.imgFormat = BufferFormatArgb;
		_screensWithWidgets.append(grabbedScreen);
	}

	return !_screensWithWidgets.isEmpty();
}

GrabResult WaylandGrabber::grabScreens()
{
	for (GrabbedScreen& screen : _screensWithWidgets) {
		WaylandGrabberData* d = reinterpret_cast<WaylandGrabberData*>(screen.associatedData);
		if (d == nullptr || !d->hasFrame) {
			return GrabResultFrameNotReady;
		}

		screen.imgData = reinterpret_cast<const unsigned char*>(d->frameBytes.constData());
		screen.imgDataSize = static_cast<size_t>(d->frameBytes.size());
		screen.bytesPerRow = static_cast<size_t>(d->bytesPerLine);
		screen.imgFormat = d->format;
	}

	return GrabResultOk;
}

void WaylandGrabber::freeScreens()
{
	for (GrabbedScreen& screen : _screensWithWidgets) {
		WaylandGrabberData* d = reinterpret_cast<WaylandGrabberData*>(screen.associatedData);
		if (d != nullptr) {
			if (d->capture)
				d->capture->setActive(false);
			delete d;
			screen.associatedData = nullptr;
		}
	}
	_screensWithWidgets.clear();
}

#endif // WAYLAND_QT_GRAB_SUPPORT
