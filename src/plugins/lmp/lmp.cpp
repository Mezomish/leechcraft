/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "lmp.h"
#include <QToolBar>
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "entitychecker.h"
#include "playerwidget.h"

using namespace LeechCraft::Plugins::LMP;
using namespace LeechCraft::Util;

void LMP::Init (ICoreProxy_ptr proxy)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("lmp"));

	SettingsDialog_.reset (new XmlSettingsDialog ());
	SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
			"lmpsettings.xml");

	Core::Instance ().SetCoreProxy (proxy);

	connect (&Core::Instance (),
			SIGNAL (bringToFront ()),
			this,
			SIGNAL (bringToFront ()));
	connect (&Core::Instance (),
			SIGNAL (gotEntity (const LeechCraft::Entity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::Entity&)));
}

void LMP::SecondInit ()
{
}

void LMP::Release ()
{
	Core::Instance ().Release ();
	XmlSettingsManager::Instance ()->Release ();
}

QByteArray LMP::GetUniqueID () const
{
	return "org.LeechCraft.LMP";
}

QString LMP::GetName () const
{
	return "LMP";
}

QString LMP::GetInfo () const
{
	return "LeechCraft Media Player";
}

QStringList LMP::Provides () const
{
	return QStringList ("media");
}

QStringList LMP::Needs () const
{
	return QStringList ();
}

QStringList LMP::Uses () const
{
	return QStringList ();
}

void LMP::SetProvider (QObject*, const QString&)
{
}

QIcon LMP::GetIcon () const
{
	return QIcon (":/plugins/lmp/resources/images/lmp.svg");
}

IVideoWidget* LMP::CreateWidget () const
{
	return Core::Instance ().CreateWidget ();
}

IVideoWidget* LMP::GetDefaultWidget () const
{
	return Core::Instance ().GetDefaultWidget ();
}

boost::shared_ptr<XmlSettingsDialog> LMP::GetSettingsDialog () const
{
	return SettingsDialog_;
}

bool LMP::CouldHandle (const LeechCraft::Entity& e) const
{
	EntityChecker ec (e);
	return ec.Can ();
}

void LMP::Handle (LeechCraft::Entity e)
{
	if (!CouldHandle (e))
		return;

	Core::Instance ().Handle (e);
}

QList<QAction*> LMP::GetActions (LeechCraft::ActionsEmbedPlace place) const
{
	QList<QAction*> result;

	if (place == AEPCommonContextMenu)
		result += Core::Instance ().GetShowAction ();

	return result;
}

Q_EXPORT_PLUGIN2 (leechcraft_lmp, LeechCraft::Plugins::LMP::LMP);

