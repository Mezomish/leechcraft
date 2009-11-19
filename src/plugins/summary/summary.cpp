/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "summary.h"
#include <QIcon>
#include <plugininterface/util.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Summary
		{
			void Summary::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (Util::InstallTranslator ("summary"));

				Core::Instance ().SetProxy (proxy);
			}

			void Summary::SecondInit ()
			{
				Core::Instance ().SecondInit ();
			}

			void Summary::Release ()
			{
				Core::Instance ().Release ();
				Translator_.reset ();
			}

			QString Summary::GetName () const
			{
				return "Summary";
			}

			QString Summary::GetInfo () const
			{
				return tr ("Summary of downloads and recent events");
			}

			QIcon Summary::GetIcon () const
			{
				return QIcon (":/resources/images/summary.svg");
			}

			QStringList Summary::Provides () const
			{
				return QStringList ();
			}

			QStringList Summary::Needs () const
			{
				return QStringList ();
			}

			QStringList Summary::Uses () const
			{
				return QStringList ();
			}

			void Summary::SetProvider (QObject*, const QString&)
			{
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_yasd, LeechCraft::Plugins::Summary::Summary);
