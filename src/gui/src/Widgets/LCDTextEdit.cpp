/*
 * Hydrogen
 * Copyright(c) 2008-2023 The hydrogen development team [hydrogen-devel@lists.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "LCDTextEdit.h"
#include "../HydrogenApp.h"
#include "../Skin.h"

#include <core/Globals.h>
#include <core/Preferences/Theme.h>

LCDTextEdit::LCDTextEdit( QWidget* pParent, bool bIsActive )
 : QTextEdit( pParent )
 , m_bEntered( false )
 , m_bIsActive( bIsActive )
{
	setReadOnly( ! bIsActive );
	setEnabled( bIsActive );
	if ( ! bIsActive ) {
		setFocusPolicy( Qt::NoFocus );
	}
	setAlignment( Qt::AlignCenter );

	const auto pPref = H2Core::Preferences::get_instance();
	
	// Derive a set of scaling-dependent font sizes on the basis of
	// the default font size determined by Qt itself.
	QFont currentFont = font();
	int nStepSize = 2;

	m_fontPointSizes.resize( 3 );
	switch ( pPref->getFontSize() ) {
	case H2Core::FontTheme::FontSize::Small:
		m_fontPointSizes[ 0 ] = currentFont.pointSize();
		break;
	case H2Core::FontTheme::FontSize::Large:
		m_fontPointSizes[ 0 ] = currentFont.pointSize() - 2 * nStepSize;
		break;
	default:
		m_fontPointSizes[ 0 ] = currentFont.pointSize() - nStepSize;
	}
	
	m_fontPointSizes[ 1 ] = m_fontPointSizes[ 0 ] + nStepSize;
	m_fontPointSizes[ 2 ] = m_fontPointSizes[ 0 ] + 2 * nStepSize;
	
	updateFont();
	updateStyleSheet();

	connect( HydrogenApp::get_instance(), &HydrogenApp::preferencesChanged,
			 this, &LCDTextEdit::onPreferencesChanged );
}

LCDTextEdit::~LCDTextEdit() {
}

void LCDTextEdit::setIsActive( bool bIsActive ) {
	m_bIsActive = bIsActive;;

	setReadOnly( ! bIsActive );
	setEnabled( bIsActive );
	
	if ( ! bIsActive ) {
		setFocusPolicy( Qt::NoFocus );
	}
	else {
		setFocusPolicy( Qt::StrongFocus );
	}

	update();
}

void LCDTextEdit::updateFont() {

	auto pPref = H2Core::Preferences::get_instance();

	int nIndex = 1;
	if ( pPref->getFontSize() == H2Core::FontTheme::FontSize::Small ) {
		nIndex = 0;
	} else if ( pPref->getFontSize() == H2Core::FontTheme::FontSize::Large ) {
		nIndex = 2;
	}

	setFontFamily( pPref->getLevel3FontFamily() );
	setFontPointSize( m_fontPointSizes[ nIndex ] );
	setPlainText( toPlainText() );
}

void LCDTextEdit::updateStyleSheet() {
	auto pPref = H2Core::Preferences::get_instance();
	
	QColor textColor = pPref->getColorTheme()->m_windowTextColor;
	QColor textColorActive = pPref->getColorTheme()->m_widgetTextColor;

	QColor backgroundColor = pPref->getColorTheme()->m_windowColor;
	QColor backgroundColorActive = pPref->getColorTheme()->m_widgetColor;

	QString sStyleSheet = QString( "\
QTextEdit:enabled { \
    color: %1; \
    background-color: %2; \
} \
QTextEdit:disabled { \
    color: %3; \
    background-color: %4; \
}" )
		.arg( textColorActive.name() )
		.arg( backgroundColorActive.name() )
		.arg( textColor.name() )
		.arg( backgroundColor.name() );

	setStyleSheet( sStyleSheet );
}

void LCDTextEdit::onPreferencesChanged( H2Core::Preferences::Changes changes ) {
	if ( changes & ( H2Core::Preferences::Changes::Colors |
					 H2Core::Preferences::Changes::Font ) ) {
		updateFont();
		updateStyleSheet();
	}
}

void LCDTextEdit::paintEvent( QPaintEvent *ev ) {

	auto pPref = H2Core::Preferences::get_instance();

	QTextEdit::paintEvent( ev );

	// Hovering highlights
	if ( m_bEntered || hasFocus() ) {
		QPainter painter( viewport() );

		QColor colorHighlightActive;
		if ( m_bIsActive ) {
			colorHighlightActive = pPref->getColorTheme()->m_highlightColor;
		} else {
			colorHighlightActive = pPref->getColorTheme()->m_lightColor;
		}

		// If the mouse is placed on the widget but the user hasn't
		// clicked it yet, the highlight will be done more transparent to
		// indicate that keyboard inputs are not accepted yet.
		if ( ! hasFocus() ) {
			colorHighlightActive.setAlpha( 150 );
		}

		QPen pen;
		pen.setColor( colorHighlightActive );
		pen.setWidth( 3 );
		painter.setPen( pen );
		painter.drawRoundedRect( QRect( 0, 0, viewport()->width() - 1, viewport()->height() - 1 ), 3, 3 );
	}
}

void LCDTextEdit::enterEvent( QEvent* ev ) {
	QTextEdit::enterEvent( ev );
	m_bEntered = true;
	update();
}

void LCDTextEdit::leaveEvent( QEvent* ev ) {
	QTextEdit::leaveEvent( ev );
	m_bEntered = false;
	update();
}
