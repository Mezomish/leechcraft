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

#include "juick.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <QCoreApplication>
#include <QIcon>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <interfaces/imessage.h>
#include <interfaces/iclentry.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Juick
{
	typedef QString& (QString::*replace_t) (const QString&, const QString&, Qt::CaseSensitivity);

	class Typo
	{
		QString Text_;
		QRegExp CheckRX_;
		QString FixPattern_;
		boost::function<QString& (QString&)> Correction_;
	public:
		Typo (const QString& text, const QString& checkPattern, const QString& fixPattern)
		: Text_ (text)
		, CheckRX_ (checkPattern)
		, FixPattern_ (fixPattern)
		{}

		Typo (const QString& text, const QString& checkPattern, 
			boost::function<QString& (QString&)> correction)
		: Text_ (text)
		, CheckRX_ (checkPattern)
		, Correction_ (correction) 
		{}

		bool Done ()
		{
			return CheckRX_.indexIn (Text_) != -1;
		}

		QString& Correction ()
		{
			if (Correction_)
				Correction_ (Text_);
			else if (CheckRX_.indexIn (Text_) != -1)
				Text_.replace (CheckRX_, FixPattern_);
			return Text_;
		}
	};

	void Plugin::Init (ICoreProxy_ptr)
	{
		UserRX_ = QRegExp ("(@[\\w\\-\\.@\\|]*)\\b([:\\s,.?!])", Qt::CaseInsensitive);
		PostRX_ = QRegExp ("<br />#(\\d+)\\s", Qt::CaseInsensitive);
		IdRX_ = QRegExp ("#(\\d+)(\\s|$|<br />)", Qt::CaseInsensitive);
		ReplyRX_ = QRegExp ("#(\\d+/\\d+)\\s?", Qt::CaseInsensitive);
		UnsubRX_ = QRegExp ("#(\\d+)/(\\d+)\\s(<a href)", Qt::CaseInsensitive);
		AvatarRX_ = QRegExp ("@([\\w\\-\\.@\\|]*):", Qt::CaseInsensitive);
	}

	void Plugin::SecondInit ()
	{
	}	

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.juick";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "juick";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("juick is plugin for nicer support of the juick.com microblogging service.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QStringList Plugin::Provides () const
	{
		return QStringList ();
	}

	QStringList Plugin::Needs () const
	{
		return QStringList ();
	}

	QStringList Plugin::Uses () const
	{
		return QStringList ();
	}

	void Plugin::SetProvider (QObject*, const QString&)
	{
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	QString Plugin::FormatBody (QString body)
	{
		body.replace (AvatarRX_, 
				"<img style='float:left;margin-right:4px' "
				"width='32px' "
				"height='32px' "
				"src='http://api.juick.com/avatar?uname=\\1&size=32'>@\\1:");
		body.replace (UserRX_,  "<a href=\"azoth://msgeditreplace/\\1+\">\\1</a>\\2");
		body.replace (PostRX_, 
				"<br /> <a href=\"azoth://msgeditreplace/%23\\1%20\">#\\1</a> "
				"("
				"<a href=\"azoth://msgeditreplace/S%20%23\\1\">S</a> "
				"<a href=\"azoth://msgeditreplace/%23\\1+\">+</a> "
				"<a href=\"azoth://msgeditreplace/!%20%23\\1\">!</a> "
				") ");
		body.replace (UnsubRX_, 
				"#\\1/\\2 (<a href=\"azoth://msgeditreplace/U%20%23\\1\">U</a>) \\3");
		body.replace (IdRX_, "<a href=\"azoth://msgeditreplace/%23\\1+\">#\\1</a>\\2");
		body.replace (ReplyRX_, "<a href=\"azoth://msgeditreplace/%23\\1%20\">#\\1</a> ");	
	
		return body;
	}


	bool Plugin::ShouldHandle (QObject* msgObj, int direction, int type)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);

		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< msgObj
				<< "to IMessage";
			return false;
		}

		if (msg->GetDirection () != direction ||
			msg->GetMessageType () != type)
		{
			return false;
		}

		ICLEntry *other = qobject_cast<ICLEntry*> (msg->OtherPart ());

		if (!other)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< msg->OtherPart ()
				<< "to ICLEntry";
			return false;
		}

		if (!other->GetEntryID ().contains ("juick@juick.com"))
			return false;

		return true;

	}

	void Plugin::hookFormatBodyEnd (IHookProxy_ptr proxy,
			QObject*, QString body, QObject *msgObj)
	{
		if(ShouldHandle (msgObj, IMessage::DIn, IMessage::MTChatMessage))
			proxy->SetValue ("body", FormatBody (body));
	}

	void Plugin::hookMessageWillCreated (LeechCraft::IHookProxy_ptr proxy, 
			QObject *chatTab, QObject *entry, int, QString, QString text)
	{
		ICLEntry *other = qobject_cast<ICLEntry*> (entry);

		if (!other)
		{
			qWarning () << Q_FUNC_INFO 
				<< "unable to cast" 
				<< entry 
				<< "to ICLEntry";
			return;
		}

		if (!other->GetEntryID ().contains ("juick@juick.com"))
			return;

		Typo typos[] = {
			Typo (text, QString::fromUtf8 ("!\\s+[#№]{2,}(\\d+)"), QString ("! #\\1")),  
			Typo (text, "!\\s+(\\d+)", QString ("! #\\1")),
			Typo (text, QString::fromUtf8 ("![#№](\\d+)"), QString ("! #\\1")),
			Typo (text, "!(\\d+)", QString ("! #\\1")),
			Typo (text, QString::fromUtf8 ("[SЫ]\\s+[#№]{2,}(\\d+)"), QString ("S #\\1")),  
			Typo (text, QString::fromUtf8 ("[SЫ]\\s+(\\d+)"), QString ("S #\\1")),
			Typo (text, QString::fromUtf8 ("[SЫ][#№](\\d+)"), QString ("S #\\1")),
			Typo (text, QString::fromUtf8 ("[SЫ](\\d+)"), QString ("S #\\1")),
			Typo (text, QString::fromUtf8 ("Ы [#№](\\d+)"), QString ("S #\\1")),
			Typo (text, QString::fromUtf8 ("ЗЬ\\s+@(.*)"), QString ("PM @\\1")),
			Typo (text, "^(\\d+)\\s+(.*)", QString ("#\\1 \\2")),
			Typo (text, "^(\\d+/\\d+)\\s+(.*)", QString ("#\\1 \\2")),
			Typo (text, QString::fromUtf8 ("^№\\+$"), QString ("#+")),
			Typo (text, QString::fromUtf8 ("^\"$"), QString ("@")),
			Typo (text, QString::fromUtf8 ("^В\\s?Д$"), QString ("D L")),
			Typo (text, QString::fromUtf8 ("$Ы^"), QString ("S")),
			Typo (text, QString::fromUtf8 ("[UГ]\\s+[#№]{2,}(\\d+)"), QString ("U #\\1")),
			Typo (text, QString::fromUtf8 ("[UГ]\\s+(\\d+)"), QString ("U #\\1")),
			Typo (text, QString::fromUtf8 ("[UГ][#№](\\d+)"), QString ("U #\\1")),
			Typo (text, QString::fromUtf8 ("[UГ](\\d+)"), QString ("U #\\1")),
			Typo (text, QString::fromUtf8 ("Г [#№](\\d+)"), QString ("U #\\1")),
			Typo (text, QString::fromUtf8 ("В [#№](\\d+)"), QString ("D #\\1")),
			Typo (text, QString::fromUtf8 ("[DВ][#№](\\d+)"), QString ("D #\\1")),
			Typo (text, QString::fromUtf8 ("[DВ](\\d+)"), QString ("D #\\1")),
			Typo (text, QString::fromUtf8 ("^РУДЗ$"), QString ("HELP")),
			Typo (text, QString::fromUtf8 ("^ДЩПШТ$"), QString ("LOGIN")),
			Typo (text, QString::fromUtf8 ("^ЩТ(\\+?)$"), QString ("ON\\1")),
			Typo (text, QString::fromUtf8 ("^ЩАА$"), QString ("OFF")),
			Typo (text, QString::fromUtf8 ("^ИД(\\s?)"), QString ("BL\\1")),
			Typo (text, QString::fromUtf8 ("^ЦД(\\s?)"), QString ("WL\\1")),
			Typo (text, QString::fromUtf8 ("^ШТМШЕУ "), QString ("INVITE ")),
			Typo (text, QString::fromUtf8 ("^МСФКВ$"), QString ("VCARD")),
			Typo (text, QString::fromUtf8 ("^ЗШТП$"), QString ("PING")),
			Typo (text, QString::fromUtf8 ("^№+$"), 
					boost::bind (
						static_cast<replace_t> (&QString::replace), 
						_1, 
						QString::fromUtf8 ("№"), 
						"#",  
						Qt::CaseInsensitive))
		};
		
		for (int i = 0; i < static_cast<int> (sizeof (typos) / sizeof (Typo)); ++i)
		{
			Typo typo = typos [i];

			if (!typo.Done ())
				continue;

			QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_AzothJuick");
			QWidget* parent = qobject_cast<QWidget*> (chatTab);
			QString correction = typo.Correction ();
			bool askForCorrection = settings.value ("AskForCorrection", true).toBool ();

			if (!parent)
			{
				qWarning () << Q_FUNC_INFO
					<< "unable to cast"
					<< chatTab
					<< "to QWidget";
				return;
			}

			QMessageBox msgbox (QMessageBox::Question, 
					tr ("Fix typo?"), 
					tr ("Did you mean <em>%1</em> instead of <em>%2</em>?")
						.arg (correction)
						.arg (text),
					QMessageBox::NoButton,
					parent);					
			msgbox.addButton (QMessageBox::Yes);
			QPushButton *always = msgbox.addButton (tr ("Always"), QMessageBox::YesRole);
			msgbox.addButton (QMessageBox::No);

			if (!askForCorrection || msgbox.exec () != QMessageBox::No)
			{
				proxy->SetValue ("text", correction);
				if (msgbox.clickedButton () == always)
					settings.setValue ("AskForCorrection", false);
			}					
			break;
		}
	}
}
}
}

Q_EXPORT_PLUGIN2 (leechcraft_azoth_juick, LeechCraft::Azoth::Juick::Plugin);
