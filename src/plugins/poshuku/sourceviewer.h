/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_POSHUKU_SOURCEVIEWER_H
#define PLUGINS_POSHUKU_SOURCEVIEWER_H
#include <QMainWindow>
#include "ui_sourceviewer.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class SourceViewer : public QMainWindow
			{
				Q_OBJECT

				Ui::SourceViewer Ui_;
			public:
				SourceViewer (QWidget* = 0);
				void SetHtml (const QString&);
			};
		};
	};
};

#endif

