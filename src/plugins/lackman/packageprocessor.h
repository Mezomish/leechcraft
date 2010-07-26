/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#ifndef PLUGINS_LACKMAN_PACKAGEPROCESSOR_H
#define PLUGINS_LACKMAN_PACKAGEPROCESSOR_H
#include <QObject>
#include <QDir>
#include <QHash>
#include <QUrl>
#include <QProcess>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			class PackageProcessor : public QObject
			{
				Q_OBJECT

				QDir DBDir_;
				QHash<QUrl, int> URL2Id_;
			public:
				PackageProcessor (QObject* = 0);

				void Remove (int);
				void Install (int);
				void Update (int);
			private slots:
				void handleResourceFetched (const QUrl&);
				void handlePackageUnarchFinished (int, QProcess::ExitStatus);
				void handleUnarchError (QProcess::ProcessError);
			private:
				/** This does all the heavy duty of installing the
				 * package identified by its id downloaded from the
				 * given url.
				 *
				 * This function expects that the package at the url is
				 * already fetched.
				 *
				 * @param[in] id The ID of the package.
				 * @param[in] url The exact URL from which the package
				 * has been downloaded.
				 */
				void HandleFile (int id, const QUrl& url);

				bool HandleEntry (int id, const QFileInfo& fi,
						const QString& stagingDir, QDir& packageDir);
				void CleanupDir (const QString&);
			signals:
				void packageInstallError (int, const QString&);
				void packageInstalled (int);
			};
		}
	}
}

#endif
