// qtractorTrackButton.cpp
//
/****************************************************************************
   Copyright (C) 2005-2024, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "qtractorAbout.h"
#include "qtractorTrackButton.h"

#include "qtractorMidiControlObserver.h"
#include "qtractorMidiControlObserverForm.h"

#include <QAction>


//----------------------------------------------------------------------------
// qtractorMidiControlButton -- MIDI controller observer tool button.

// Constructor.
qtractorMidiControlButton::qtractorMidiControlButton ( QWidget *pParent )
	: qtractorObserverWidget<QPushButton> (pParent), m_pMidiControlAction(nullptr)

{
	QPushButton::setFocusPolicy(Qt::NoFocus);
//	QPushButton::setToolButtonStyle(Qt::ToolButtonTextOnly);
	QPushButton::setCheckable(true);
}


// MIDI controller/observer attachment (context menu) activator.
void qtractorMidiControlButton::addMidiControlAction (
	qtractorMidiControlObserver *pMidiObserver )
{
	if (m_pMidiControlAction)
		removeAction(m_pMidiControlAction);

	m_pMidiControlAction
		= qtractorMidiControlObserverForm::addMidiControlAction(
			this, this, pMidiObserver);
}


void qtractorMidiControlButton::midiControlActionSlot (void)
{
	qtractorMidiControlObserverForm::midiControlAction(
		this, qobject_cast<QAction *> (sender()));
}


void qtractorMidiControlButton::midiControlMenuSlot ( const QPoint& pos )
{
	qtractorMidiControlObserverForm::midiControlMenu(
		qobject_cast<QWidget *> (sender()), pos);
}


//----------------------------------------------------------------------------
// qtractorTrackButton -- Track tool button (observer).

// Constructor.
qtractorTrackButton::qtractorTrackButton ( qtractorTrack *pTrack,
	qtractorTrack::ToolType toolType, QWidget *pParent )
	: qtractorMidiControlButton(pParent)
{
	m_pTrack   = pTrack;
	m_toolType = toolType;

	switch (m_toolType) {
	case qtractorTrack::Record:
		QPushButton::setText("R");
		m_rgbOn = Qt::red;
		break;
	case qtractorTrack::Mute:
		QPushButton::setText("M");
		m_rgbOn = Qt::yellow;
		break;
	case qtractorTrack::Solo:
		QPushButton::setText("S");
		m_rgbOn = Qt::cyan;
		break;
	}

	updateTrack(); // Visitor setup.

	QObject::connect(this, SIGNAL(toggled(bool)), SLOT(toggledSlot(bool)));
}


// Destructor.
qtractorTrackButton::~qtractorTrackButton (void)
{
	qtractorMidiControl *pMidiControl = qtractorMidiControl::getInstance();
	if (pMidiControl == nullptr)
		return;

	qtractorMidiControlObserver *pMidiObserver = nullptr;

	switch (m_toolType) {
	case qtractorTrack::Record:
		pMidiObserver = m_pTrack->recordObserver();
		break;
	case qtractorTrack::Mute:
		pMidiObserver = m_pTrack->muteObserver();
		break;
	case qtractorTrack::Solo:
		pMidiObserver = m_pTrack->soloObserver();
		break;
	}

	if (pMidiObserver)
		pMidiControl->unmapMidiObserverWidget(pMidiObserver, this);
}


// Visitors overload.
void qtractorTrackButton::updateValue ( float fValue )
{
	const bool bBlockSignals = QPushButton::blockSignals(true);

	QPalette pal;
	const QColor rgbText = pal.buttonText().color();
	const QColor rgbOff = pal.button().color();
	const bool bOn = (fValue > 0.0f);
	pal.setColor(QPalette::ButtonText, bOn ? m_rgbOn.darker() : rgbText);
	pal.setColor(QPalette::Button, bOn ? m_rgbOn : rgbOff);
	QPushButton::setPalette(pal);
	QPushButton::setChecked(bOn);

	QPushButton::blockSignals(bBlockSignals);
}


// Special toggle slot.
void qtractorTrackButton::toggledSlot ( bool bOn )
{
	// Just emit proper signal...
	m_pTrack->stateChangeNotify(m_toolType, bOn);
}


// Specific accessors.
void qtractorTrackButton::setTrack ( qtractorTrack *pTrack )
{
	m_pTrack = pTrack;

	updateTrack();
}

qtractorTrack *qtractorTrackButton::track (void) const
{
	return m_pTrack;
}


qtractorTrack::ToolType qtractorTrackButton::toolType (void) const
{
	return m_toolType;
}


// Track state (record, mute, solo) button setup.
void qtractorTrackButton::updateTrack (void)
{
	qtractorMidiControlObserver *pMidiObserver = nullptr;

	switch (m_toolType) {
	case qtractorTrack::Record:
		setSubject(m_pTrack->recordSubject());
		pMidiObserver = m_pTrack->recordObserver();
		break;
	case qtractorTrack::Mute:
		setSubject(m_pTrack->muteSubject());
		pMidiObserver = m_pTrack->muteObserver();
		break;
	case qtractorTrack::Solo:
		setSubject(m_pTrack->soloSubject());
		pMidiObserver = m_pTrack->soloObserver();
		break;
	}

	if (pMidiObserver) {
		pMidiObserver->setCurveList(m_pTrack->curveList());
		addMidiControlAction(pMidiObserver);
	}

	observer()->update(true);
}


// Refresh color (palette) state buttons
void qtractorTrackButton::updateTrackButton (void)
{
	updateValue(isChecked() ? 1.0f : 0.0f);
}


// end of qtractorTrackButton.cpp
