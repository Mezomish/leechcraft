/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "lackman.h"
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QIcon>
#include <plugininterface/util.h>
#include <plugininterface/tagscompleter.h>
#include <plugininterface/categoryselector.h>
#include "core.h"
#include "packagesdelegate.h"
#include "pendingmanager.h"
#include "typefilterproxymodel.h"
#include "xmlsettingsmanager.h"
#include "packagesmodel.h"
#include "externalresourcemanager.h"
#include "storage.h"
#include "stringfiltermodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (Util::InstallTranslator ("lackman"));

				Ui_.setupUi (this);
				
				TagsModel_ = new QStringListModel (this);
				Util::TagsCompleter *tc = new Util::TagsCompleter (Ui_.SearchLine_);
				tc->OverrideModel (TagsModel_);
				Ui_.SearchLine_->AddSelector ();

				SettingsDialog_.reset (new Util::XmlSettingsDialog ());
				SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"lackmansettings.xml");

				Core::Instance ().SetProxy (proxy);
				Core::Instance ().FinishInitialization ();

				SettingsDialog_->SetDataSource ("RepositoryList",
						Core::Instance ().GetRepositoryModel ());

				connect (&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				connect (&Core::Instance (),
						SIGNAL (tagsUpdated (const QStringList&)),
						this,
						SLOT (handleTagsUpdated (const QStringList&)));
				connect (&Core::Instance (),
						SIGNAL (tagsUpdated (const QStringList&)),
						Ui_.SearchLine_,
						SLOT (handleTagsUpdated (const QStringList&)));

				TypeFilter_ = new TypeFilterProxyModel (this);
				TypeFilter_->setDynamicSortFilter (true);
				TypeFilter_->setSourceModel (Core::Instance ().GetPluginsModel ());
				FilterString_ = new StringFilterModel (this);
				FilterString_->setDynamicSortFilter (true);
				FilterString_->setFilterCaseSensitivity (Qt::CaseInsensitive);
				FilterString_->setSourceModel (TypeFilter_);

				Ui_.PackagesTree_->setModel (FilterString_);
				PackagesDelegate *pd = new PackagesDelegate (Ui_.PackagesTree_);
				Ui_.PackagesTree_->setItemDelegate (pd);

				Ui_.PendingTree_->setModel (Core::Instance ()
						.GetPendingManager ()->GetPendingModel ());

				connect (Ui_.PackagesTree_->selectionModel (),
						SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
						pd,
						SLOT (handleRowChanged (const QModelIndex&, const QModelIndex&)),
						Qt::QueuedConnection);
				connect (Ui_.PackagesTree_->selectionModel (),
						SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
						this,
						SLOT (handlePackageSelected (const QModelIndex&)));
				connect (Ui_.SearchLine_,
						SIGNAL (textChanged (const QString&)),
						FilterString_,
						SLOT (setFilterFixedString (const QString&)));
				
				connect (Core::Instance ().GetPendingManager (),
						SIGNAL (fetchListUpdated (const QList<int>&)),
						this,
						SLOT (handleFetchListUpdated (const QList<int>&)));

				BuildActions ();

				const QStringList& tags = Core::Instance ().GetAllTags ();
				handleTagsUpdated (tags);
				Ui_.SearchLine_->handleTagsUpdated (tags);
				handleFetchListUpdated (QList<int> ());
			}

			void Plugin::SecondInit ()
			{
				QList<IWebBrowser*> browsers = Core::Instance ().GetProxy ()->
						GetPluginsManager ()->GetAllCastableTo<IWebBrowser*> ();
				if (browsers.size ())
					Ui_.Browser_->Construct (browsers.at (0));
				Ui_.Browser_->SetNavBarVisible (false);
				Ui_.Browser_->SetEverythingElseVisible (false);

				Core::Instance ().SecondInit ();
			}

			void Plugin::Release ()
			{
				Core::Instance ().Release ();
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.LackMan";
			}

			QString Plugin::GetName () const
			{
				return "LackMan";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("LeechCraft Package Manager.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/resources/images/lackman.svg");
			}

			QWidget* Plugin::GetTabContents ()
			{
				return this;
			}

			QToolBar* Plugin::GetToolBar () const
			{
				return Toolbar_;
			}

			Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
			{
				return SettingsDialog_;
			}

			QList<QAction*> Plugin::GetActions (ActionsEmbedPlace place) const
			{
				QList<QAction*> result;
				if (place == AEPToolsMenu)
				{
					result << UpdateAll_;
					result << UpgradeAll_;
				}
				return result;
			}

			void Plugin::handleTagsUpdated (const QStringList& tags)
			{
				TagsModel_->setStringList (tags);
			}

			void Plugin::on_PackageStatus__currentIndexChanged (int index)
			{
				TypeFilter_->SetFilterMode (static_cast<TypeFilterProxyModel::FilterMode> (index));
			}
			
			namespace
			{
				void AddText (QString& text, const QStringList& urls)
				{
					Q_FOREACH (const QString& url, urls)
					{
						boost::optional<QByteArray> opt =
								Core::Instance ().GetExtResourceManager ()->
										GetResourceData (QUrl::fromEncoded (url.toUtf8 ()));
						if (!opt || opt->isEmpty ())
							continue;

						text += QString ("<img src='data:image/png;base64,%1' alt='' /><br />")
								.arg (QString (opt->toBase64 ()));
					}
				}
			}
			
			void Plugin::handlePackageSelected (const QModelIndex& index)
			{
				QString text;
				AddText (text, index.data (PackagesModel::PMRThumbnails).toStringList ());
				text += index.data (PackagesModel::PMRLongDescription).toString ();
				AddText (text, index.data (PackagesModel::PMRScreenshots).toStringList ());
				
				Ui_.Browser_->SetHtml (text);

				if (index.isValid ())
					Ui_.PackageInfoBox_->setTitle (tr ("Package information: %1").arg (index.data ().toString ()));
				else
					Ui_.PackageInfoBox_->setTitle (tr ("Package information"));

				QString state;
				if (!index.isValid ()) ;
				else if (!index.data (PackagesModel::PMRInstalled).toBool ())
					state = tr ("not installed");
				else if (index.data (PackagesModel::PMRUpgradable).toBool ())
					state = tr ("installed; upgradable");
				else
					state = tr ("installed");
				Ui_.StateLabel_->setText (state);
				
				const qint64 size = index.data (PackagesModel::PMRSize).toLongLong ();
				const QString& sizeText = size >= 0 ?
						Util::MakePrettySize (size) :
						tr ("unknown");
				Ui_.SizeLabel_->setText (sizeText);
			}
			
			void Plugin::handleFetchListUpdated (const QList<int>& ids)
			{
				qint64 sumSize = 0;
				Q_FOREACH (const int id, ids)
				{
					if (Core::Instance ().GetListPackageInfo (id).IsInstalled_)
						continue;
					
					const qint64 size = Core::Instance ().GetStorage ()->GetPackageSize (id);
					if (size > 0)
						sumSize += size;
				}
				
				if (sumSize > 0)
					Ui_.TotalSizeLabel_->setText (tr ("Total size to be downloaded: %1")
								.arg (Util::MakePrettySize (sumSize)));
				Ui_.TotalSizeLabel_->setVisible (sumSize > 0);
			}

			void Plugin::BuildActions ()
			{
				UpdateAll_ = new QAction (tr ("Update all repos"), this);
				UpdateAll_->setProperty ("ActionIcon", "refresh");
				connect (UpdateAll_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (updateAllRequested ()));

				UpgradeAll_ = new QAction (tr ("Upgrade all packages"), this);
				UpgradeAll_->setProperty ("ActionIcon", "fetchall");
				connect (UpgradeAll_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (upgradeAllRequested ()));
				
				Apply_ = new QAction (tr ("Apply"), this);
				Apply_->setProperty ("ActionIcon", "apply");
				connect (Apply_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (acceptPending ()));
				
				Cancel_ = new QAction (tr ("Cancel"), this);
				Cancel_->setProperty ("ActionIcon", "cancel");
				connect (Cancel_,
						SIGNAL (triggered ()),
						&Core::Instance (),
						SLOT (cancelPending ()));

				Toolbar_ = new QToolBar (GetName ());
				Toolbar_->addAction (UpdateAll_);
				Toolbar_->addAction (UpgradeAll_);
				Toolbar_->addSeparator ();
				Toolbar_->addAction (Apply_);
				Toolbar_->addAction (Cancel_);
			}
		}
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_lackman, LeechCraft::Plugins::LackMan::Plugin);
