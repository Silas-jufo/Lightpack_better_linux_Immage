/*
 * WaylandGrabber.hpp
 *
 *  Created on: 04.10.2026
 *      Project: Lightpack
 */

#pragma once

#include "GrabberBase.hpp"

#ifdef WAYLAND_QT_GRAB_SUPPORT

class WaylandGrabber : public GrabberBase
{
	Q_OBJECT
public:
	WaylandGrabber(QObject *parent, GrabberContext *context);
	virtual ~WaylandGrabber();

	DECLARE_GRABBER_NAME("WaylandGrabber")

	virtual void startGrabbing() override;
	virtual void stopGrabbing() override;

protected:
	virtual GrabResult grabScreens() override;
	virtual bool reallocate(const QList<ScreenInfo> &screens) override;
	virtual QList<ScreenInfo> *screensWithWidgets(QList<ScreenInfo> *result, const QList<GrabWidget *> &grabWidgets) override;

private:
	void freeScreens();
};

#endif // WAYLAND_QT_GRAB_SUPPORT
