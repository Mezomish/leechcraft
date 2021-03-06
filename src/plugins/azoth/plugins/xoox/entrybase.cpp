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

#include "entrybase.h"
#include <QImage>
#include <QStringList>
#include <QtDebug>
#include <QXmppVCardIq.h>
#include <QXmppPresence.h>
#include <plugininterface/util.h>
#include <interfaces/iproxyobject.h>
#include <interfaces/azothutil.h>
#include "glooxmessage.h"
#include "glooxclentry.h"
#include "glooxprotocol.h"
#include "vcarddialog.h"
#include "glooxaccount.h"
#include "clientconnection.h"
#include "util.h"
#include "core.h"
#include <QXmppClient.h>
#include <QXmppRosterManager.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	EntryBase::EntryBase (GlooxAccount *parent)
	: QObject (parent)
	, Account_ (parent)
	{

	}

	QObject* EntryBase::GetObject ()
	{
		return this;
	}

	QList<QObject*> EntryBase::GetAllMessages () const
	{
		return AllMessages_;
	}
	
	void EntryBase::PurgeMessages (const QDateTime& before)
	{
		Azoth::Util::StandardPurgeMessages (AllMessages_, before);
	}

	EntryStatus EntryBase::GetStatus (const QString& variant) const
	{
		if (!variant.isEmpty () &&
				CurrentStatus_.contains (variant))
			return CurrentStatus_ [variant];

		if (CurrentStatus_.size ())
			return *CurrentStatus_.begin ();

		return EntryStatus ();
	}

	QList<QAction*> EntryBase::GetActions () const
	{
		return Actions_;
	}

	QImage EntryBase::GetAvatar () const
	{
		return Avatar_;
	}

	QString EntryBase::GetRawInfo () const
	{
		return RawInfo_;
	}

	void EntryBase::ShowInfo ()
	{
		if (Account_->GetState ().State_ == SOffline)
		{
			Entity e = LeechCraft::Util::MakeNotification ("Azoth",
					tr ("Can't view info while offline"),
					PCritical_);
			Core::Instance ().SendEntity (e);

			return;
		}

		if (!VCardDialog_)
			VCardDialog_ = new VCardDialog ();

		Account_->GetClientConnection ()->RequestInfo (GetJID ());
		Account_->GetClientConnection ()->FetchVCard (GetJID (), VCardDialog_);
		VCardDialog_->show ();
	}

	QMap<QString, QVariant> EntryBase::GetClientInfo (const QString& var) const
	{
		return Variant2ClientInfo_ [var];
	}

	void EntryBase::HandleMessage (GlooxMessage *msg)
	{
		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		IProxyObject *proxy = qobject_cast<IProxyObject*> (proto->GetProxyObject ());
		proxy->PreprocessMessage (msg);

		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void EntryBase::UpdateChatState (QXmppMessage::State state, const QString& variant)
	{
		emit chatPartStateChanged (static_cast<ChatPartState> (state), variant);
	}

	void EntryBase::SetStatus (const EntryStatus& status, const QString& variant)
	{
		const bool existed = CurrentStatus_.contains (variant);
		const bool wasOffline = existed ?
				CurrentStatus_ [variant].State_ == SOffline :
				false;
		if (existed &&
				status == CurrentStatus_ [variant])
			return;

		CurrentStatus_ [variant] = status;
		emit statusChanged (status, variant);
		
		if (!existed ||
				(existed && status.State_ == SOffline) ||
				wasOffline)
			emit availableVariantsChanged (Variants ());
		
		if (status.State_ != SOffline)
		{
			QXmppRosterManager& rm = Account_->
					GetClientConnection ()->GetClient ()->rosterManager ();
			const QMap<QString, QXmppPresence>& presences =
					rm.getAllPresencesForBareJid (GetJID ());
			if (presences.contains (variant))
			{
				const int p = presences.value (variant).status ().priority ();
				Variant2ClientInfo_ [variant] ["priority"] = p;
			}
		}
		
		GlooxMessage *message = new GlooxMessage (IMessage::MTStatusMessage,
				IMessage::DIn,
				GetJID (),
				variant,
				Account_->GetClientConnection ().get ());
		message->SetMessageSubType (IMessage::MSTParticipantStatusChange);

		GlooxProtocol *proto = qobject_cast<GlooxProtocol*> (Account_->GetParentProtocol ());
		IProxyObject *proxy = qobject_cast<IProxyObject*> (proto->GetProxyObject ());
		const QString& state = proxy->StateToString (status.State_);
		
		const QString& nick = GetEntryName () + '/' + variant;
		message->setProperty ("Azoth/Nick", nick);
		message->setProperty ("Azoth/TargetState", state);
		message->setProperty ("Azoth/StatusText", status.StatusString_);

		QString msg = tr ("%1 is now %2 (%3)")
				.arg (nick)
				.arg (state)
				.arg (status.StatusString_);
		message->SetBody (msg);
		HandleMessage (message);
	}

	void EntryBase::SetAvatar (const QByteArray& data)
	{
		if (!data.size ())
			SetAvatar (QImage ());
		else
			SetAvatar (QImage::fromData (data));
	}

	void EntryBase::SetAvatar (const QImage& avatar)
	{
		Avatar_ = avatar;

		emit avatarChanged (Avatar_);
	}

	void EntryBase::SetVCard (const QXmppVCardIq& vcard)
	{
		QString text = FormatRawInfo (vcard);
		if (!text.isEmpty ())
			text = QString ("gloox VCard:\n") + text;
		SetRawInfo (text);
		SetAvatar (vcard.photo ());

		if (VCardDialog_)
			VCardDialog_->UpdateInfo (vcard);
	}

	void EntryBase::SetRawInfo (const QString& rawinfo)
	{
		RawInfo_ = rawinfo;

		emit rawinfoChanged (RawInfo_);
	}

	void EntryBase::SetClientInfo (const QString& variant,
			const QString& node)
	{
		QString type = Util::GetClientIDName (node);
		if (type.isEmpty ())
		{
			if (!node.isEmpty ())
				qWarning () << Q_FUNC_INFO
						<< "unknown client type for"
						<< node;
			type = "unknown";
		}
		Variant2ClientInfo_ [variant] ["client_type"] = type;

		QString name = Util::GetClientHRName (node);
		if (name.isEmpty ())
		{
			if (!node.isEmpty ())
				qWarning () << Q_FUNC_INFO
						<< "unknown client name for"
						<< node;
			name = "Unknown";
		}
		Variant2ClientInfo_ [variant] ["client_name"] = name;
	}

	void EntryBase::SetClientInfo (const QString& variant, const QXmppPresence& pres)
	{
		SetClientInfo (variant, pres.capabilityNode ());
	}

	QString EntryBase::FormatRawInfo (const QXmppVCardIq& vcard)
	{
		QString text;
		text += tr ("Name: %1")
				.arg (vcard.fullName ());
		text += "\n";

		if (vcard.nickName ().size ())
			text += tr ("Nickname: %1\n")
					.arg (vcard.nickName ());
		if (vcard.url ().size ())
			text += tr ("URL: %1\n")
					.arg (vcard.url ());
		if (vcard.birthday ().isValid ())
			text += tr ("Birthday: %1\n")
					.arg (vcard.birthday ().toString ());
		if (vcard.email ().size ())
			text += tr ("Email: %1\n")
					.arg (vcard.email ());

		if (vcard.photoType ().size ())
		{
			text += tr ("Photo:") + QString ("\ndata:%1;base64,%2\n")
						.arg (vcard.photoType ())
						.arg (vcard.photo ().constData ());
		}

		return text;
	}
}
}
}
