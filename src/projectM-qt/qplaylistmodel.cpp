/**
 * projectM-qt -- Qt4 based projectM GUI 
 * Copyright (C)2003-2004 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */

#include "projectM.h"

#include <QIcon>
#include <QXmlStreamReader>
#include <QtDebug>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>

#include "qxmlplaylisthandler.hpp"
#include "qplaylistmodel.hpp"
#include <QMimeData>

QString QPlaylistModel::PRESET_MIME_TYPE("text/x-projectM-preset");

class XmlReadFunctor {
	public:
		XmlReadFunctor(QPlaylistModel & model) : m_model(model) {}

		inline void setPlaylistDesc(const QString & desc) {
			m_model.setPlaylistDesc(desc);
		}

		inline void appendItem(const QString & url, const QString & name, int rating, int breedability) {
			m_model.appendRow(url, name, rating, breedability);
		}
		

	private:
		QPlaylistModel & m_model;

};


class XmlWriteFunctor {
	public:
		XmlWriteFunctor(QPlaylistModel & model) : m_model(model), m_index(0) {}


		inline const QString & playlistDesc() const {
			return m_model.playlistDesc();
		}

		inline bool nextItem(QString & name, QString & url, int & rating, int & breedability) {
			Q_UNUSED(name);

			if (m_index >= m_model.rowCount())
				return false;

			QModelIndex modelIndex = m_model.index(m_index, 0);
			url = m_model.data(modelIndex, QPlaylistModel::URLInfoRole).toString();
			rating = m_model.data(modelIndex, QPlaylistModel::RatingRole).toInt();

			breedability = m_model.data(modelIndex, QPlaylistModel::BreedabilityRole).toInt();
			
			m_index++;
			return true;
		}
	private:
		QPlaylistModel & m_model;
		int m_index;

};

bool QPlaylistModel::softCutRatingsEnabled() const {
    auto projectMSettings = projectm_get_settings(m_projectM);
    auto softCutRatingsEnabledValue = projectMSettings->soft_cut_ratings_enabled;
    projectm_free_settings(projectMSettings);
    return softCutRatingsEnabledValue;
}

QPlaylistModel::QPlaylistModel ( projectm* _projectM, QObject * parent ) :
		QAbstractTableModel ( parent ), m_projectM ( _projectM )
{

}


void QPlaylistModel::updateItemHighlights()
{
	if ( rowCount() == 0 )
		return;

	emit ( dataChanged ( this->index ( 0,0 ), this->index ( rowCount()-1,columnCount()-1 ) ) );
}

bool QPlaylistModel::setData ( const QModelIndex & index, const QVariant & value, int role )
{
	if ( role == QPlaylistModel::RatingRole )
	{
        projectm_set_preset_rating(m_projectM, index.row(), value.toInt(), PROJECTM_HARD_CUT_RATING_TYPE);
		return true;
	} else if (role == QPlaylistModel::BreedabilityRole) {
        projectm_set_preset_rating(m_projectM, index.row(), value.toInt(), PROJECTM_SOFT_CUT_RATING_TYPE);
		return true;
	} else if (role == QPlaylistModel::NameRole) {
		projectm_set_preset_name(m_projectM, index.row(), value.toString().toLocal8Bit().data());
		return true;
	}
	else
		return QAbstractTableModel::setData ( index, value, role );

}

Qt::ItemFlags QPlaylistModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
	
	if (index.isValid())
		return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
	else
		return Qt::ItemIsDropEnabled | defaultFlags;
}


 bool QPlaylistModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
				int row, int column, const QModelIndex &parent)
{
	Q_UNUSED(row);
	Q_UNUSED(parent);
	if (!data->hasFormat(PRESET_MIME_TYPE))
		return false;

	if (action == Qt::IgnoreAction)
		return true;

	if (column > 0)
		return false;
	
	return true;
}

QVariant QPlaylistModel::ratingToIcon ( int rating )  const
{
	switch ( rating )
	{
		case 1:
			return QVariant ();
		case 2:
			return QVariant ( QIcon ( ":/images/icons/rating-1.png" ) );
		case 3:
			return QVariant ( QIcon ( ":/images/icons/rating-2.png" ) );
		case 4:
			return QVariant ( QIcon ( ":/images/icons/rating-3.png" ) );
		case 5:
			return QVariant ( QIcon ( ":/images/icons/rating-4.png" ) );
		case 6:
			return QVariant ( QIcon ( ":/images/icons/rating-5.png" ) );
		default:
			if (rating <= 0)
				return QVariant ();
			else 
				return QVariant ( QIcon ( ":/images/icons/rating-5.png" ) );
	}
}


QVariant QPlaylistModel::breedabilityToIcon( int rating )  const
{
	switch ( rating )
	{
		case 1:
			return QVariant ();
		case 2:
			return QVariant ( QIcon ( ":/images/icons/rating-1.png" ) );
		case 3:
			return QVariant ( QIcon ( ":/images/icons/rating-2.png" ) );
		case 4:
			return QVariant ( QIcon ( ":/images/icons/rating-3.png" ) );
		case 5:
			return QVariant ( QIcon ( ":/images/icons/rating-4.png" ) );
		case 6:
			return QVariant ( QIcon ( ":/images/icons/rating-5.png" ) );
		default:
			if (rating <= 0)
				return QVariant ();
			else 
				return QVariant ( QIcon ( ":/images/icons/rating-5.png" ) );
	}
}

QString QPlaylistModel::getSillyRatingToolTip(int rating) {
	
switch (rating) {
	case 1:
		return QString("Rather watch grass grow than watch this");
	case 2:
		return QString("A very poor preset");
	case 3:
		return QString("Tolerable");
	case 4:
		return QString("Pretty good");
	case 5:
		return QString("Trippy eye candy");
	case 6:		
		return QString("Crafted by a psychotic deity");
	default:
		if (rating <= 0 )
			return QString("So bad it literally makes other presets bad!");
		else
			return QString("Better than projectM itself!");
}
}


QString QPlaylistModel::getBreedabilityToolTip(int rating) const {
	
switch (rating) {
	case 1:
		return QString("Hidious.");
	case 2:
		return QString("Ugly.");
	case 3:
		return QString("Doable.");
	case 4:
		return QString("Hot.");
	case 5:
		return QString("Preset Magnet.");
	case 6:		
		return QString("Preset Whore.");
	default:
		if (rating <= 0 )
			return QString("Infertile.");
		else
			return QString("Diseased slut.");
}
}

QVariant QPlaylistModel::data ( const QModelIndex & index, int role = Qt::DisplayRole ) const
{

	if (!index.isValid())
		return QVariant();
	
	unsigned int pos;
	const int rowidx = index.row();

	switch ( role )
	{
		case Qt::DisplayRole:
			if ( index.column() == 0 )
            {
			    auto presetName = projectm_get_preset_name(m_projectM, rowidx);
			    QString qPresetName = QString(presetName);
                projectm_free_string(presetName);
                return qPresetName;
            }
			else if (index.column() == 1)
			    return ratingToIcon (projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_HARD_CUT_RATING_TYPE) );
			else
			    return ratingToIcon ( projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_SOFT_CUT_RATING_TYPE) );
		case Qt::ToolTipRole:
			if ( index.column() == 0 )
			{
			    auto presetName = projectm_get_preset_name(m_projectM, rowidx);
			    QString qPresetName = QString(presetName);
			    projectm_free_string(presetName);
			    return qPresetName;
			}
			else if (index.column() == 1)
			    return QString ( getSillyRatingToolTip(projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_HARD_CUT_RATING_TYPE)));
			else return 		getBreedabilityToolTip(projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_SOFT_CUT_RATING_TYPE));
		case Qt::DecorationRole:
			if ( index.column() == 1 )
				return ratingToIcon (projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_HARD_CUT_RATING_TYPE) );
			else if (index.column() == 2)
			    return breedabilityToIcon ( projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_SOFT_CUT_RATING_TYPE) );
			else
				return QVariant();
		case QPlaylistModel::RatingRole:
		    return QVariant (  projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_HARD_CUT_RATING_TYPE)  );
		case QPlaylistModel::BreedabilityRole:
		    return QVariant (  projectm_get_preset_rating(m_projectM, rowidx, PROJECTM_SOFT_CUT_RATING_TYPE)  );
		case Qt::BackgroundRole:
	
			if (!projectm_get_selected_preset_index(m_projectM, &pos))
				return QVariant();
			if (projectm_is_preset_locked(m_projectM) && rowidx >= 0 && static_cast<unsigned int>(rowidx) == pos )
				return QColor(Qt::red);
			if (!projectm_is_preset_locked(m_projectM) && rowidx >= 0 && static_cast<unsigned int>(rowidx) == pos )
				return QColor(Qt::green);
			return QVariant();
		case QPlaylistModel::URLInfoRole:
        {
            auto presetUrl = projectm_get_preset_filename(m_projectM, rowidx);
            QString qPresetUrl(presetUrl);
            projectm_free_string(presetUrl);
            return qPresetUrl;
        }
		default:
			
			return {};
	}
}

QVariant QPlaylistModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{

	if ( orientation == Qt::Vertical )
		return QAbstractTableModel::headerData ( section, orientation, role );
	if ( ( section == 0 ) && ( role == Qt::DisplayRole ) )
		return QString ( tr ( "Preset" ) );

	/// @bug hack. this should be formalized like it is in libprojectM.
	if ( ( section == 1 ) && ( role == Qt::DisplayRole )) {
		if (columnCount() == 2)
			return QString ( tr ( "Rating" ) );
		else
			return QString ( tr ( "Hard Rating" ) );
    }
	if ( ( section == 2 ) && ( role == Qt::DisplayRole ) )
		return QString ( tr ( "Soft Rating" ) );

	return QAbstractTableModel::headerData ( section, orientation, role );
}

int QPlaylistModel::rowCount ( const QModelIndex & parent ) const
{
	Q_UNUSED(parent);

	return projectm_get_playlist_size(m_projectM);
}


int QPlaylistModel::columnCount ( const QModelIndex & parent ) const
{
	Q_UNUSED(parent);

	return softCutRatingsEnabled() ? 3 : 2;
}

void QPlaylistModel::appendRow ( const QString & presetURL, const QString & presetName, int rating, int breedability )
{
    auto ratings = new int[2]{ rating, breedability };

	beginInsertRows ( QModelIndex(), rowCount(), rowCount() );
	projectm_add_preset_url(m_projectM, presetURL.toLocal8Bit().data(), presetName.toLocal8Bit().data(), ratings, 2);
	endInsertRows();

	delete[] ratings;
}

void QPlaylistModel::insertRow (int index, const QString & presetURL, const QString & presetName, int rating, int breedability)  {
    auto ratings = new int[2]{ rating, breedability };

	beginInsertRows ( QModelIndex(), index, index);
	projectm_insert_preset_url(m_projectM, index, presetURL.toLocal8Bit().data(), presetName.toLocal8Bit().data(), ratings, 2);
	endInsertRows();
}

bool QPlaylistModel::removeRows ( int row, int count, const QModelIndex & parent)   {
	Q_UNUSED(parent);
	
	beginRemoveRows ( QModelIndex(), row, count);

	for (int i = 0; i < count; i++) {
        projectm_remove_preset(m_projectM, row);
	}
	endRemoveRows();
	return true;
}

bool QPlaylistModel::removeRow ( int index, const QModelIndex & parent)
{
	Q_UNUSED(parent);

	beginRemoveRows ( QModelIndex(), index, index );
	projectm_remove_preset(m_projectM, index);
	endRemoveRows();
	return true;
}

void QPlaylistModel::clear()
{
	clearItems();
	m_playlistName = "";
	m_playlistDesc = "";
}

void QPlaylistModel::clearItems()
{
    if (rowCount() > 0)
    {
        beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
        projectm_clear_playlist(m_projectM);
        endRemoveRows();
    }
}


bool QPlaylistModel::writePlaylist ( const QString & file ) {

	QFile qfile(file);

 	if (!qfile.open(QIODevice::WriteOnly)) {
		QMessageBox::warning (0, "Playlist Save Error", QString("There was a problem trying to save the playlist \"%1\".  You may not have permission to modify this file.").arg(file));
		return false;
	}

	XmlWriteFunctor writeFunctor(*this);

	QXmlPlaylistHandler::writePlaylist(&qfile, writeFunctor);
	return true;
}

bool QPlaylistModel::readPlaylist ( const QString & file )
{
	
	if (QFileInfo(file).isDir()) {
		if (!QDir(file).isReadable()) {
			QMessageBox::warning (0, "Playlist Directory Error", QString(tr("There was a problem trying to open the playlist directory \"%1\".  The directory doesn't exist or you may not have permission to open it. ")).arg(file));
			return false;
		}
		
		foreach (QFileInfo info, QDir(file).entryInfoList()) {
			if (info.fileName().toLower().endsWith(".prjm") || info.fileName().toLower().endsWith(".milk") || info.fileName().toLower().endsWith(".so"))
				appendRow(info.absoluteFilePath(), info.fileName(), 3,3);
		}
		return true;
	}
	
        QFile qfile(file);
 	if (!qfile.open(QIODevice::ReadOnly)) {
		QMessageBox::warning (0, "Playlist File Error", QString(tr("There was a problem trying to open the playlist \"%1\".  The file may no longer exist or you may not have permission to read the file.")).arg(file));
		return false;
	}

	XmlReadFunctor readFunctor(*this);

	if (QXmlPlaylistHandler::readPlaylist(&qfile, readFunctor) != QXmlStreamReader::NoError) {
		QMessageBox::warning ( 0, "Playlist Parse Error", QString(tr("There was a problem trying to parse the playlist \"%1\". Some of your playlist items may have loaded correctly, but don't expect miracles.")).arg(file));
	}

	return true;
}

void QPlaylistModel::notifyDataChanged(unsigned int ind)
{


	QModelIndex modelIndex = index (ind, 1);
	emit ( dataChanged ( modelIndex, modelIndex));
}

