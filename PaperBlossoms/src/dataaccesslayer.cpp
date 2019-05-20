/*
 * *******************************************************************
 * This file is part of the Paper Blossoms application
 * (https://github.com/dashnine/PaperBlossoms).
 * Copyright (c) 2019 Kyle Hankins (dashnine)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * The Legend of the Five Rings Roleplaying Game is the creation
 * and property of Fantasy Flight Games.
 * *******************************************************************
 */

#include "dataaccesslayer.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>
#include <QCoreApplication>
#include <QSqlRecord>
#include <QDir>
#include <QSqlTableModel>

DataAccessLayer::DataAccessLayer(QString locale)
{
    QString targetpath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if(!QDir(targetpath).exists()){
        QDir().mkdir(targetpath);
    }
    const QString appdir = QCoreApplication::applicationDirPath();
    targetpath += "/paperblossoms.db";
    qDebug() << targetpath;
    //check filemodtime
    const QFileInfo ri(":/data/paperblossoms.db");
    //QFileInfo ri(appdir + "/paperblossoms.db");
    const QFileInfo fi(targetpath);

    qDebug() << "resource: "+ ri.lastModified().toString();
    qDebug() << "localfile: "+ fi.lastModified().toString();
            //copy db to standardpaths

    if(ri.lastModified()>fi.lastModified()){
        QMessageBox msgBox;
        msgBox.setText("The local data is missing or older than the bundled data.");
        msgBox.setInformativeText("Do you want to overwrite local data?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        const int ret = msgBox.exec();

        if(ret == QMessageBox::Yes){
            QFile::remove(targetpath);
        }
    }

    //implicitly fails if the file already exists at the target!
    QFile::copy(":/data/paperblossoms.db", targetpath);
    QFile::setPermissions(targetpath, QFile::WriteOwner | QFile::ReadOwner);
    //QFile::copy(appdir + "/paperblossoms.db", targetpath);



    //connect to DB

    const QString DRIVER("QSQLITE");
    if(QSqlDatabase::isDriverAvailable(DRIVER)){
          QSqlDatabase db = QSqlDatabase::addDatabase(DRIVER);
            //db.setDatabaseName(":memory:");
          //db.setDatabaseName("testdb.db");
          //db.setDatabaseName("paperblossoms.db");
          db.setDatabaseName(targetpath);


        if(!db.open())
            qWarning() << "ERROR: " << db.lastError();
    }

    //import translation table for locale (if possible)
    importCSV(":/translations/data/i18n/i18n_"+locale+".csv","i18n",false);
    //:/translations/data/i18n/i18n_en.csv

}

QString DataAccessLayer::untranslate(QString string_tr){
    QSqlQuery query;
    query.prepare("SELECT string FROM i18n WHERE string_tr = ?");
    query.bindValue(0, string_tr);
    query.exec();
    while (query.next()) {
        QString out = query.value(0).toString();
        return out;
    }
    qWarning() << "ERROR - Untranslated value for "+ string_tr+" not found.";
    return "";
}

QString DataAccessLayer::translate(QString string){
    QSqlQuery query;
    query.prepare("SELECT string_tr FROM i18n WHERE string = ?");
    query.bindValue(0, string);
    query.exec();
    while (query.next()) {
        QString out = query.value(0).toString();
        return out;
    }
    qWarning() << "ERROR - Translated value for "+ string +" not found. Using original.";
    return string;
}

QStringList DataAccessLayer::qsl_getclans()
{
    QStringList out;
    //clan query
    QSqlQuery query("SELECT name_tr FROM clans ORDER BY name_tr");
    while (query.next()) {
        const QString cname = query.value(0).toString();
        out << cname;
//        qDebug() << cname;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getfamilies(const QString clan)
{
    QStringList out;
    //family query
    QSqlQuery query;
    query.prepare("SELECT name_tr FROM families WHERE clan_tr = :clan ORDER BY name_tr");
    query.bindValue(0, clan);
    query.exec();
    while (query.next()) {
        const QString fname = query.value(0).toString();
        out << fname;
//        qDebug() << fname;
    }
    return out;
}

QString DataAccessLayer::qs_getclandesc(const QString clan)
{
    QSqlQuery query;
    query.prepare("SELECT description FROM clans WHERE name_tr = :clan");
    query.bindValue(0, clan);
    query.exec();
    while (query.next()) {
        const QString desc = query.value(0).toString();
//        qDebug() << desc;
        return desc;
    }
    qWarning() << "ERROR - Clan" + clan + " not found while searching for desc.";
    return "";
}

QString DataAccessLayer::qs_getclanref(const QString clan)
{
    QSqlQuery query;
    query.prepare("SELECT reference_book, reference_page FROM clans WHERE name_tr = :clan");
    query.bindValue(0, clan);
    query.exec();
    while (query.next()) {
        QString ref = query.value(0).toString() + " ";
        ref += query.value(1).toString();
//        qDebug() << desc;
        return ref;
    }
    qWarning() << "ERROR - Clan" + clan + " not found while searching for desc.";
    return "";
}

QString DataAccessLayer::qs_getfamilydesc(const QString family)
{
    QSqlQuery query;
    query.prepare("SELECT description FROM families WHERE name_tr = :family");
    query.bindValue(0, family);
    query.exec();
    while (query.next()) {
        const QString desc = query.value(0).toString();
//        qDebug() << desc;
        return desc;
    }
    qWarning() << "ERROR - Family" + family + " not found while searching for desc.";
    return "";
}

QString DataAccessLayer::qs_getfamilyref(const QString family)
{
    QSqlQuery query;
    query.prepare("SELECT reference_book, reference_page FROM families WHERE name_tr = :family");
    query.bindValue(0, family);
    query.exec();
    while (query.next()) {
        QString ref = query.value(0).toString() + " ";
        ref += query.value(1).toString();
//        qDebug() << desc;
        return ref;
    }
    qWarning() << "ERROR - Family" + family + " not found while searching for desc.";
    return "";
}

QStringList DataAccessLayer::qsl_getfamilyrings(const QString fam ){
    //bonus query - rings, skills
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT ring_tr FROM family_rings WHERE family_tr = :family");
    query.bindValue(0, fam);
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
//        qDebug() << cname;
        out<< cname;
    }
    //model->setQuery(query);
    return out;
}

QStringList DataAccessLayer::qsl_getschools(const QString clan, const bool allclans ){
    QStringList out;
    QSqlQuery query;
    if(!allclans){
        query.prepare("SELECT name_tr FROM schools WHERE clan_tr = :clan");
        query.bindValue(0, clan);
    }
    else{
        query.prepare("SELECT name_tr FROM schools");
    }
    query.exec();
    while (query.next()) {
        const QString result = query.value(0).toString();
//        qDebug() << result;
        out<< result;
    }
    return out;

}

//TODO: this is a QStringList, but only returns 1 skill right now.  Refactor?
QStringList DataAccessLayer::qsl_getclanskills(const QString clan ){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT skill_tr FROM clans WHERE name_tr = :clan");
    query.bindValue(0, clan);
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
//        qDebug() << cname;
        out<< cname;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getfamilyskills(const QString family ){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT skill_tr FROM family_skills WHERE family_tr = :family");
    query.bindValue(0, family);
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
        out<< cname;
    }
    return out;
}

QString DataAccessLayer::qs_getschooldesc(const QString school ){
    QString out;
    QSqlQuery query;
        query.prepare("SELECT description FROM schools WHERE name_tr = :school");
        query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        const QString result = query.value(0).toString();
//        qDebug() << result;
        out = result;
    }
    return out;

}

QString DataAccessLayer::qs_getringdesc(const QString ring ){
    QString out;
    QSqlQuery query;
        query.prepare("SELECT outstanding_quality_tr FROM rings WHERE name_tr = :ring");
        query.bindValue(0, ring);
    query.exec();
    while (query.next()) {
        const QString result = query.value(0).toString();
//        qDebug() << result;
        out = result;
    }
    return out;

}

QStringList DataAccessLayer::qsl_getdescribablenames()
{
    QStringList out;
    QSqlQuery query;
    query.prepare(
                "           select name                    FROM advantages_disadvantages       "
                "UNION      SELECT name                    FROM armor                          "
                "UNION      SELECT name                    FROM clans                          "
                "UNION      SELECT name                    FROM families                       "
                "UNION      SELECT name                    FROM personal_effects               "
                "UNION      SELECT quality                 FROM qualities                      "
                "UNION      SELECT name                    FROM schools                        "
                "UNION      SELECT school_ability_name     FROM schools                        "
                "UNION      SELECT mastery_ability_name    FROM schools                        "
                "UNION      SELECT name                    FROM techniques                     "
                "UNION      SELECT name                    FROM titles                         "
                "UNION      SELECT title_ability_name      FROM titles                         "
                "UNION      SELECT name                    FROM weapons                        "

                );
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
        out<< name;
    }
    return out;
}

QString DataAccessLayer::qs_getschooladvdisadv(const QString school ){
    QString out;
    QSqlQuery query;
        query.prepare("SELECT advantage_disadvantage_tr FROM schools WHERE name_tr = :school");
        query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        const QString result = query.value(0).toString();
//        qDebug() << result;
        out = result;
    }
    return out;

}

QString DataAccessLayer::qs_getschoolref(const QString school)
{
    QSqlQuery query;
    query.prepare("SELECT reference_book, reference_page FROM schools WHERE name_tr = :school");
    query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        QString ref = query.value(0).toString() + " ";
        ref += query.value(1).toString();
//        qDebug() << desc;
        return ref;
    }
    qWarning() << "ERROR - School" + school + " not found while searching for desc.";
    return "";
}


QStringList DataAccessLayer::qsl_getschoolskills(const QString school ){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT skill_tr FROM school_starting_skills WHERE school_tr = :school");
    query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
//        qDebug() << cname;
        out<< cname;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getskills(){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT skill_tr FROM skills ");
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
        out<< cname;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getskillsandgroup(){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT skill_tr, skill_group_tr FROM skills ");
    query.exec();
    while (query.next()) {
        const QString s = query.value(0).toString()+"|"+query.value(1).toString();
        out<< s;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getskillsbygroup(const QString group){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT skill_tr FROM skills WHERE skill_group_tr = ?");
    query.bindValue(0, group);
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
        out<< cname;
    }
    return out;
}

int DataAccessLayer::i_getschoolskillcount(const QString school ){
    QSqlQuery query;
    query.prepare("SELECT starting_skills_size FROM schools WHERE name_tr = :school");
    query.bindValue(0, school);
    query.exec();
    int count = 0;
    while (query.next()) {
        count = query.value(0).toString().toInt();
//        qDebug() << count;
    }
    return count;
}
/*
int DataAccessLayer::i_getschooltechcount(const QString school){
    QSqlQuery query;
    query.prepare("SELECT count(distinct set_id) FROM school_starting_techniques WHERE school = :school");
    query.bindValue(0, school);
    query.exec();
    int count = 0;
    while (query.next()) {
        count = query.value(0).toString().toInt();
//        qDebug() << count;
    }
    return count;

}
*/
QStringList DataAccessLayer::qsl_getschooltechsetids(const QString school){
    QSqlQuery query;
    QStringList out;
    query.prepare("SELECT distinct set_id FROM school_starting_techniques WHERE school_tr = :school");
    query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
    }
    return out;

}

QStringList DataAccessLayer::qsl_getschoolequipsetids(const QString school){
    QSqlQuery query;
    QStringList out;
    query.prepare("SELECT distinct set_id FROM school_starting_outfit WHERE school_tr = :school");
    query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
    }
    return out;

}

//returns a qlist of qstringlists.  Each list starts with a selection count, followed by a list of options
QList<QStringList> DataAccessLayer::ql_getlistsoftech(const QString school)
{
    //get the number of batches
    const QStringList techids = qsl_getschooltechsetids(school);
    QList<QStringList> out;
    foreach (const QString id, techids) {

        QSqlQuery query;
        query.prepare("SELECT set_size, technique_tr FROM school_starting_techniques WHERE school_tr = :school and set_id = :id");
        query.bindValue(0, school);
        query.bindValue(1, id);
        query.exec();
        //TODO - better way of handling these lists? feels hacky
        //first entry: number of things to select.
        //remaining entries: selection set for that row
        //UI will create NUMBER comboboxes containing SELECTIONSET items.
        int choice_count = 0;
        QStringList techlist;
        while (query.next()) {
            QStringList list;
            choice_count = query.value(0).toString().toInt();
            techlist << query.value(1).toString();
        }
        QStringList row;
        row.append( QString::number(choice_count));
        row.append(techlist);
        out << row;

    }
//    qDebug() << out;
    return out;

}

//returns a qlist of qstringlists.  Each list starts with a selection count, followed by a list of options
QList<QStringList> DataAccessLayer::ql_getlistsofeq(const QString school)
{

    //get the number of batches
    QStringList ids = qsl_getschoolequipsetids(school);
    QList<QStringList> out;
    foreach (QString id, ids) {

        QSqlQuery query;
        query.prepare("SELECT set_size, equipment_tr FROM school_starting_outfit WHERE school_tr = :school and set_id = :id");
        query.bindValue(0, school);
        query.bindValue(1, id);
        query.exec();
        //TODO - better way of handling these lists? feels hacky
        //first entry: number of things to select.
        //remaining entries: selection set for that row
        //UI will create NUMBER comboboxes containing SELECTIONSET items.
        int choice_count = 0;
        QStringList eqlist;
        while (query.next()) {
            QStringList list;
            choice_count = query.value(0).toString().toInt();
            eqlist << query.value(1).toString();
        }
        QStringList row;
        row.append( QString::number(choice_count));
        row.append(eqlist);
        out << row;

    }
//    qDebug() << out;
    return out;

   }

/* // TODO - adapt this to handle it all with one query?
QStringList DataAccessLayer::qsl_getstartingeqfixed(QString school){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT startinggear FROM schools WHERE name = :school");
    query.bindValue(0, school);
    query.exec();
    //TODO - replace with multi-line response in special table, rather than splitting?
    while (query.next()) {
        QString name = query.value(0).toString();
        qDebug() << name;
        out<< name.split(", ");
    }
    return out;
}
*/

QStringList DataAccessLayer::qsl_getrings( ){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT name_tr FROM rings");
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getadvdisadv(const QString category ){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT name_tr FROM advantages_disadvantages WHERE category = :category");
    query.bindValue(0, category);
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getadvdisadvbyname(const QString name ){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT category, name_tr, ring_tr, description, short_desc, reference_book, reference_page, types FROM advantages_disadvantages WHERE name_tr = :name");
    query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
        out << query.value(1).toString();
        out << query.value(2).toString();
        out << query.value(3).toString();
        out << query.value(4).toString();
        out << query.value(5).toString();
        out << query.value(6).toString();
        out << query.value(7).toString();
//        qDebug() << name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getadv(){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT name_tr FROM advantages_disadvantages WHERE category IN ('Distinctions', 'Passions')");
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}
QStringList DataAccessLayer::qsl_getdisadv(){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT name_tr FROM advantages_disadvantages WHERE category IN ('Adversities', 'Anxieties')");
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getitemsunderrarity(const int rarity ){
    //bonus query - rings, skills
    QStringList out;
    QSqlQuery query;
    query.prepare("select distinct name_tr from personal_effects where rarity <= ? union select distinct name_tr from weapons "
                  "where rarity <= ? union select distinct name_tr from armor where rarity <= ?");
        query.bindValue(0, rarity);
        query.bindValue(1, rarity);
        query.bindValue(2, rarity);
    query.exec();
    //TODO - replace with multi-line response in special table, rather than splitting?
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getweaponsunderrarity(const int rarity ){
    //bonus query - rings, skills
    QStringList out;
    QSqlQuery query;
    query.prepare("select distinct name_tr from weapons "
                  "where rarity <= ?");
        query.bindValue(0, rarity);
    query.exec();
    //TODO - replace with multi-line response in special table, rather than splitting?
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getweapontypeunderrarity(const int rarity, const QString type ){
    //bonus query - rings, skills
    QStringList out;
    QSqlQuery query;
    query.prepare("select distinct name_tr from weapons "
                  "where rarity <= ?                 "
                  "and category = ?                  ");
        query.bindValue(0, rarity);
        query.bindValue(1, type);
    query.exec();
    //TODO - replace with multi-line response in special table, rather than splitting?
    while (query.next()) {
        QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getitemsbytype(const QString type ){
    //bonus query - rings, skills
    QStringList out;
    QSqlQuery query;
    if(type == "Weapon"){
        query.prepare("select distinct name_tr from weapons");
    }
    else if (type == "Armor"){
        query.prepare("select name_tr from armor");

    }
    else{
        query.prepare("select name_tr from personal_effects");

    }
    query.exec();
    //TODO - replace with multi-line response in special table, rather than splitting?
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getancestors(QString source){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT ancestor_tr FROM samurai_heritage WHERE source = ? order by roll_min");
    query.bindValue(0, source);
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getancestormods(const QString ancestor){

    QMap<QString, int> map;
    QStringList out;

    map["Honor"] = 0;
    map["Glory"] = 0;
    map["Status"] = 0;
    QSqlQuery query;
    query.prepare("SELECT modifier_honor, modifier_glory, modifier_status FROM samurai_heritage WHERE ancestor_tr = :ancestor");
    query.bindValue(0, ancestor);
    query.exec();
    while (query.next()) {
        map["Honor"] = query.value(0).toInt();
        map["Glory"] = query.value(1).toInt();
        map["Status"] = query.value(2).toInt();
    }
    out.append(QString::number(map["Honor"]));
    out.append(QString::number(map["Glory"]));
    out.append(QString::number(map["Status"]));
    return out;
}




QStringList DataAccessLayer::qsl_getancestorseffects(const QString ancestor){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT outcome_tr FROM heritage_effects where ancestor_tr = :ancestor order by roll_min");
    query.bindValue(0, ancestor);
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_gettechbytyperank(const QString type, const int rank){
    QStringList out;
    QSqlQuery query;
    query.prepare("select name_tr from techniques where category = :type and rank <= :rank");
    query.bindValue(0, type);
    query.bindValue(1, rank);
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getmahoninjutsu(const int rank){
    QStringList out;
    QSqlQuery query;
    query.prepare("select name_tr from techniques where category IN ('Mahō', 'Ninjutsu') and rank <= :rank");
    query.bindValue(0, rank);
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QString DataAccessLayer::qs_getclanring(const QString clan)
{
    QSqlQuery query;
    query.prepare("SELECT ring_tr FROM clans WHERE name_tr = :clan");
    query.bindValue(0, clan);
    query.exec();
    while (query.next()) {
        const QString ring = query.value(0).toString();
//        qDebug() << ring;
        return ring;
    }
    qWarning() << "ERROR - Clan" + clan + " not found while searching for rings.";
    return "";
}

QStringList DataAccessLayer::qsl_getschoolrings(const QString school ){
    QStringList out;
    QSqlQuery query;
        query.prepare("SELECT ring_tr FROM school_rings WHERE school_tr = :school");
        query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        const QString result = query.value(0).toString();
//        qDebug() << result;
        out<< result;
    }
    return out;

}
QStringList DataAccessLayer::qsl_getqualities(){
    QStringList out;
    QSqlQuery query;
    query.prepare("select quality_tr from qualities");
    query.exec();
    while (query.next()) {
        const QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getpatterns(){
    QStringList out;
    QSqlQuery query;
    query.prepare("select name_tr from item_patterns");
    query.exec();
    while (query.next()) {
        QString name = query.value(0).toString();
//        qDebug() << name;
        out<< name;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getheritageranges(const QString heritage){
    QStringList out;
    QSqlQuery query;
        query.prepare("SELECT roll_min, roll_max from HERITAGE_EFFECTS where ancestor_tr = :heritage ORDER BY roll_min");
        query.bindValue(0, heritage);
    query.exec();
    while (query.next()) {
        const QString a = query.value(0).toString();
        const QString b = query.value(1).toString();
//        qDebug() << a + ", " + b;
        out << a + ", " + b;
    }
    return out;
}

QStringList DataAccessLayer::qsl_getancestorranges(const QString source){
    QStringList out;
    QSqlQuery query;
        query.prepare("SELECT roll_min, roll_max from samurai_heritage where source = ? ORDER BY roll_min");
        query.bindValue(0, source);
    query.exec();
    while (query.next()) {
        QString a = query.value(0).toString();
        QString b = query.value(1).toString();
//        qDebug() << a + ", " + b;
        out << a + ", " + b;
    }
    return out;
}

int DataAccessLayer::i_getclanstatus(const QString clan){
    int out = 0;
    QSqlQuery query;
    query.prepare("SELECT status FROM clans WHERE name_tr = :clan");
    query.bindValue(0, clan);
    query.exec();
    while (query.next()) {
        out = query.value(0).toInt();
    }
    return out;
}

int DataAccessLayer::i_getfamilyglory(const QString family){
    int out = 0;
    QSqlQuery query;
    query.prepare("SELECT glory FROM families WHERE name_tr = :family");
    query.bindValue(0, family);
    query.exec();
    while (query.next()) {
        out = query.value(0).toInt();
    }
    return out;
}

int DataAccessLayer::i_getfamilywealth(const QString family){
    int out = 0;
    QSqlQuery query;
    query.prepare("SELECT wealth FROM families WHERE name_tr = :family");
    query.bindValue(0, family);
    query.exec();
    while (query.next()) {
        out = query.value(0).toInt();

    }
    return out;
}

int DataAccessLayer::i_getschoolhonor(const QString school){
    int out = 0;
    QSqlQuery query;
    query.prepare("SELECT honor FROM schools WHERE name_tr = :school");
    query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        out = query.value(0).toInt();
    }
    return out;
}

QMap<QString, int> DataAccessLayer::qm_heritagehonorglorystatus(const QString heritage){

    QMap<QString, int> map;
    QStringList out;

    map["Honor"] = 0;
    map["Glory"] = 0;
    map["Status"] = 0;
    QSqlQuery query;
    query.prepare("SELECT modifier_honor, modifier_glory, modifier_status FROM samurai_heritage WHERE ancestor_tr = :ancestor");
    query.bindValue(0, heritage);
    query.exec();
    while (query.next()) {
        map["Honor"] = query.value(0).toInt();
        map["Glory"] = query.value(1).toInt();
        map["Status"] = query.value(2).toInt();
    }
    return map;


}

/*
QStringList DataAccessLayer::qsl_getschooltechavailable(QString school, bool maho_allowed ){
    QStringList out;
    QSqlQuery query;
        query.prepare("SELECT technique FROM school_techniques_available WHERE school = :school");
        query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        QString result = query.value(0).toString();
        out<< result;
    }
    return out;

}
*/

QStringList DataAccessLayer::qsl_gettechbyname(const QString name ){
    QStringList out;
    QSqlQuery query;
        query.prepare(
        "SELECT distinct name_tr, category, subcategory, rank,                                         "
        "       reference_book, reference_page,restriction_tr,                                         "
        "       short_desc, description                                                             "
        "FROM techniques                                                                            "
        "WHERE name_tr = :name                                                                         "
        );
        query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
        out << query.value(1).toString();
        out << query.value(2).toString();
        out << query.value(3).toString();
        out << query.value(4).toString();
        out << query.value(5).toString();
        out << query.value(6).toString();
        out << query.value(7).toString();
        out << query.value(8).toString();
    }
    return out;

}

QString DataAccessLayer::getLastExecutedQuery(const QSqlQuery& query)
{
 QString str = query.executedQuery();
 QMapIterator<QString, QVariant> it(query.boundValues());
 while (it.hasNext())
 {
  it.next();
  str.replace(it.key(),it.value().toString());
 }
 return str;
}

void DataAccessLayer::qsm_gettechniquetable(QSqlQueryModel * const model, const QString rank, const QString school, const QString title, const QString clan)
{
    //technique query
    //assembles the technique options from four sources:
    //      Techs, sorted by category, that are allowed due to rank, filtered by restriciton (school, clan)
    //      Techs allowed due to individual special access (title and curric)
    //      Techs allowed due to tech group special acces (title and curric)
    //      NOTE: checks for subcategory spec access (i.e. water shuji) AND category special access (i.e. Kata)

    const int trank = i_gettitletechgrouprank(title);

    QSqlQuery query;
    query.prepare(


                    "SELECT distinct name_tr, category, subcategory, rank,                                         "
                    "       xp, reference_book, reference_page,restriction_tr                                          "
                    "FROM techniques                                                                            "//First, get title special
                    "WHERE                                                                                      "//  group techs
                //---------------------Special access groups and katagroups from title-------------------//
                    "(rank <= ? and subcategory in                                                              " //0 trank
                    " (SELECT name from title_advancements                                                      "
                    "   WHERE title_tr = ?                                                                         " //1 title
                    "   AND type = 'technique_group'                                                            "
                    "   AND special_access = 1                                                                  "
                    "  )  )                                                                                     "
                    "OR                                                                                         " //title Katas (cat v subcat)
                    "(rank <= ? and category in                                                                 " //2 trank //cat is group
                    " (SELECT name from title_advancements                                                      "
                    "   WHERE title_tr = ?                                                                         " //3 title
                    "   and type = 'technique_group'                                                            "
                    "   AND special_access = 1                                                                  "
                    "  ) )                                                                                      "
                //----------------------Special access groups and katagroups from curriculum---------------//
                    "OR ( rank <= ? and subcategory in                                                          " //4 rank
                    " (SELECT advance from curriculum                                                           "
                    "   WHERE school_tr = ?                                                                        " //5 school //subcat is group
                    "   AND rank = ? and type = 'technique_group'                                               " //6 rank
                    "   AND special_access = 1                                                                  "
                    " )                                                                                         "
                    "OR  rank <= ? and category in                                                              " //7 rank
                    " (SELECT advance from curriculum                                                           "
                    "   WHERE school_tr = ?                                                                        " //8 school //subcat is group
                    "   AND rank = ? and type = 'technique_group'                                               " //9 rank
                    "   AND special_access = 1                                                                  "
                    " )  )                                                                                      "
                //------------------------special access indiv tech from curric and title------------------//
                    "OR name_tr in (                                                                               "
                    "  SELECT advance_tr from curriculum                                                           "
                    "   WHERE school_tr = ?                                                                        " //10 school //tech
                    "   AND rank = ?                                                                            " //11 rank
                    "   AND type = 'technique'                                                                  "
                    "   AND special_access = 1                                                                  "
                    "  )                                                                                        "
                    "OR name_tr in (                                                                               "           //title tech
                    "  SELECT name_tr from title_advancements                                                      "
                    "   WHERE title_tr = ?                                                                         " //12 title
                    "   AND type = 'technique'                                                                  "
                    "   AND special_access = 1                                                                  "
                    "  )                                                                                        "
                //--------------------------normal tech groups access, restrictions apply-------------------//
                    "OR                                                                                         "
                    "(rank <= ?                                                                                 " //13 rank   //tech group
                    "   AND (category in                                                                         "
                    "   (SELECT technique from school_techniques_available                                      "
                    "       WHERE school_tr = ?)                                                                   " //14 school
                    "   OR subcategory in                                                                         "
                    "   (SELECT technique from school_techniques_available                                      "
                    "       WHERE school_tr = ?))                                                                   " //15 school
                    ")                                                                                          "
                //--------------------------MAHO (and patterns and scrolls FOR EVERYONE!--------------------//
                    "OR                                                                                         "
                    "((rank <= ? OR rank = NULL)                                                                                " //16 rank
                    "   AND category IN ('Mahō', 'Item Patterns', 'Signature Scrolls')                                                                   "
                    ")                                                                                          "
                    "ORDER BY category, rank, name                                                              "
                    "");

        //also get title tech special access

        query.bindValue(0, trank);
        query.bindValue(1, title);
        query.bindValue(2, trank);
        query.bindValue(3, title);
        query.bindValue(4, rank);
        query.bindValue(5, school);
        query.bindValue(6, rank);
        query.bindValue(7, rank);
        query.bindValue(8, school);
        query.bindValue(9, rank);
        query.bindValue(10, school);
        query.bindValue(11, rank);
        query.bindValue(12, title);
        query.bindValue(13, rank);
        //query.bindValue(14, clan);
        //query.bindValue(15, school);
        query.bindValue(14, school);
        query.bindValue(15, school);
        query.bindValue(16, rank);
        query.exec();
        qDebug() << getLastExecutedQuery(query);
    while (query.next()) {
        const QString cname = query.value(0).toString();
//        qDebug() << cname;
    }
    model->setQuery(query);

}

/*
void DataAccessLayer::qsm_getfilteredtechniquetable(QSqlQueryModel * const model, const QString category, const QString rank, const QString school)
{
    //technique query
    //assembles the technique options from three sources:
    //      Techs, sorted by category, that are allowed due to rank,
    //      Techs allowed due to individual special access
    //      Techs allowed due to tech group special access

    //TODO: add title special access?



    QSqlQuery query;
    query.prepare(  "SELECT name, category, subcategory, rank, reference_book, reference_page                   " //select main list
                    "FROM techniques                                                                            " // from table
                    "WHERE category = ? and name in (                                                           " //
                    "    SELECT advance from curriculum                                                         " //spec access for cat
                    "     WHERE school = ? AND rank = ? AND type = 'technique' and special_access = 1           " //
                    "    )                                                                                      " //
                    "OR category = ? AND rank <= ?                                                              " //regular for cat
                    "OR category = ? and subcategory in                                                         " //
                    " (SELECT advance from curriculum                                                           " //
                    "   WHERE school = ?                                                                        " //tech_grp spec_access
                    "   AND rank = ? and type = 'technique_group'                                               " // for category
                    "   AND special_access = 1                                                                  "
                    " )                                                                                         "
                    "ORDER BY category, rank, name                                                              "
                    );
        query.bindValue(0, category);
        query.bindValue(1, school);
        query.bindValue(2, rank);
        query.bindValue(3, category);
        query.bindValue(4, rank);
        query.bindValue(5, category);
        query.bindValue(6, school);
        query.bindValue(7, rank);
        query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
        qDebug() << cname;
    }
    model->setQuery(query);

}
*/
void DataAccessLayer::qsm_getschoolcurriculum(QSqlQueryModel * const model, const QString school)
{

    QSqlQuery query;
    query.prepare(  "SELECT rank, advance_tr, type, special_access                  " //select main list
                    "FROM curriculum                                             " // from table
                    "WHERE school_tr = ?                                            "
                    );
        query.bindValue(0, school);
        query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
//        qDebug() << cname;
    }
    model->setQuery(query);

}
/*
void DataAccessLayer::qsm_getschoolcurriculumbyrank(QSqlQueryModel * const model, const QString school, const int rank)
{

    QSqlQuery query;
    query.prepare(  "SELECT rank, advance, type, special_access                  " //select main list
                    "FROM curriculum                                             " // from table
                    "WHERE school = ? and rank = ?                                           "
                    );
        query.bindValue(0, school);
        query.bindValue(1, rank);
        query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
//        qDebug() << cname;
    }
    model->setQuery(query);

}
*/
QStringList DataAccessLayer::qsl_gettechbygroup(const QString group,const int rank){
    //have to use Like here, since the subcategory for Kata is 'General Kata' or 'Close Combat Kata'
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT name_tr FROM techniques WHERE subcategory LIKE ? and rank <= ?");
    query.bindValue(0, QString("%%1%").arg(group));
    query.bindValue(1, rank);
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
        out<< cname;
    }
    return out;
}

QStringList DataAccessLayer::qsl_gettitles(){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT name_tr FROM titles ");
    query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
        out<< cname;
    }
    return out;
}

QString DataAccessLayer::qs_gettitleref(const QString title){
    QString out = "";
    QSqlQuery query;
    query.prepare("SELECT reference_book, reference_page FROM titles where name_tr = ?");
    query.bindValue(0, title);
    query.exec();
    while (query.next()) {
        const QString b = query.value(0).toString();
        const QString p = query.value(1).toString();
        out += (b + " " + p);
    }
    return out;
}

QString DataAccessLayer::qs_gettitlexp(const QString title){
    QString out = "";
    QSqlQuery query;
    query.prepare("SELECT xp_to_completion FROM titles where name_tr = ?");
    query.bindValue(0, title);
    query.exec();
    while (query.next()) {
        const QString b = query.value(0).toString();
        out = b;
    }
    return out;
}

QString DataAccessLayer::qs_gettitleability(const QString title){
    QString out = "";
    QSqlQuery query;
    query.prepare("SELECT title_ability_name_tr FROM titles where name_tr = ?");
    query.bindValue(0, title);
    query.exec();
    while (query.next()) {
        const QString b = query.value(0).toString();
        out = b;
    }
    return out;
}
/*
void DataAccessLayer::qsm_gettitletrack(QSqlQueryModel * const model, const QString title)
{

    QSqlQuery query;
    query.prepare(  "SELECT title, name, type, special_access,rank           " //select main list
                    "FROM title_advancements                                     " // from table
                    "WHERE title = ?                                             "
                    );
        query.bindValue(0, title);
        query.exec();
    while (query.next()) {
        const QString cname = query.value(0).toString();
//        qDebug() << cname;
    }
    model->setQuery(query);

}
*/
QStringList DataAccessLayer::qsl_gettitletrack(const QString title)
{
    QStringList out;
    QSqlQuery query;
    query.prepare(  "SELECT title_tr, name_tr, type, special_access,rank           " //select main list
                    "FROM title_advancements                                     " // from table
                    "WHERE title_tr = ?                                             "
                    );
        query.bindValue(0, title);
        query.exec();
    while (query.next()) {
        const QString title = query.value(0).toString();
        const QString name = query.value(1).toString();
        const QString type = query.value(2).toString();
        const QString sp_ac = query.value(3).toString();
        const QString rank = query.value(4).toString();
        out<< title+"|"+name+"|"+type+"|"+sp_ac+"|"+rank;
//        qDebug() << out;
    }
    return out;
}

int DataAccessLayer::i_gettitletechgrouprank(const QString title){
    int out = 0;
    QSqlQuery query;
    query.prepare(  "SELECT rank                                                 " //select main list
                    "FROM title_advancements                                     " // from table
                    "WHERE title_tr = ?                                             "
                    );
        query.bindValue(0, title);
        query.exec();
    while (query.next()) {
        if( !query.value(0).isNull())
            out = query.value(0).toInt();
//        qDebug() << out;
    }
    return out;
}

QString DataAccessLayer::qs_getitemtype(const QString name){
    QSqlQuery query;
    query.prepare("select count(distinct name_tr) from weapons where name_tr = ?");
        query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        const int count = query.value(0).toInt();
        if (count>0) return "Weapon";
    }
    query.clear();
    query.prepare("select count(distinct name_tr) from armor where name_tr = ?");
        query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        const int count = query.value(0).toInt();
        if (count>0) return "Armor";
    }
    query.clear();
    query.prepare("select count(distinct name_tr) from personal_effects where name_tr = ?");
        query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        const int count = query.value(0).toInt();
        if (count>0) return "Personal Effect";
    }


    return "Unknown";
}

/*
QStringList DataAccessLayer::qsl_getweapon(const QString name){
    //      0       1               2           3               4                   5               6           7
    //    name  |description    |short_desc |reference_book |reference_page    |price_value    |price_unit |rarity |
    //      8       9       10          11          12      13          14
    //    skill  |grip   |range_min  |range_max  |damage |deadliness | qualities
    //                          15                  16
    //    (qualities)| resistance_category | resist_value
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT name, description short_desc, reference_book, reference_page, price_value, price_unit, rarity       "
                  ",skill, grip, range_min, range_max, damage, deadliness                                               "
                  "from weapons where name = ?                                                                      ");
        query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        QString str = "WEAPON|";
        str += query.value(0).toString()+"|";
        str += query.value(1).toString()+"|";
        str += query.value(2).toString()+"|";
        str += query.value(3).toString()+"|";
        str += query.value(4).toString()+"|";
        str += query.value(5).toString()+"|";
        str += query.value(6).toString()+"|";
        str += query.value(7).toString()+"|";
        str += query.value(8).toString()+"|";
        str += query.value(9).toString()+"|";
        str += query.value(10).toString()+"|";
        str += query.value(11).toString()+"|";
        str += query.value(12).toString()+"|";
        str += query.value(13).toString()+"|";
        out << str;
    }
    return out;
}
*/
/*
QString DataAccessLayer::qs_getarmor(const QString name){
    //      0       1               2           3               4                   5               6           7
    //    name  |description    |short_desc |reference_book |reference_page    |price_value    |price_unit |rarity |
    //      8       9       10          11          12      13          14
    //    skill  |grip   |range_min  |range_max  |damage |deadliness | qualities
    //                          15                  16
    //    (qualities)| resistance_category | resist_value
    QString out;
    QSqlQuery query;
    query.prepare("SELECT name, description short_desc, reference_book, reference_page, price_value, price_unit, rarity "
                  //",skill, grip, range_min, range_max, damage, deadliness "
                  "from armor where name = ?");
        query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        QString str = "ARMOR|";
        str += query.value(0).toString()+"|";
        str += query.value(1).toString()+"|";
        str += query.value(2).toString()+"|";
        str += query.value(3).toString()+"|";
        str += query.value(4).toString()+"|";
        str += query.value(5).toString()+"|";
        str += query.value(6).toString()+"|";
        str += query.value(7).toString()+"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        out = str;
    }
    return out;
}
*/
/*
QString DataAccessLayer::qs_getperseffects(const QString name){
    //      0       1               2           3               4                   5               6           7
    //    name  |description    |short_desc |reference_book |reference_page    |price_value    |price_unit |rarity |
    //      8       9       10          11          12      13          14
    //    skill  |grip   |range_min  |range_max  |damage |deadliness | qualities
    //                          15                  16
    //    (qualities)| resistance_category | resist_value
    QString out;
    QSqlQuery query;
    query.prepare("SELECT name, description short_desc, reference_book, reference_page, price_value, price_unit, rarity "
                  //",skill, grip, range_min, range_max, damage, deadliness "
                  "from personal_effects where name = ?");
        query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        QString str = "PERSONAL_EFFECT|";
        str += query.value(0).toString()+"|";
        str += query.value(1).toString()+"|";
        str += query.value(2).toString()+"|";
        str += query.value(3).toString()+"|";
        str += query.value(4).toString()+"|";
        str += query.value(5).toString()+"|";
        str += query.value(6).toString()+"|";
        str += query.value(7).toString()+"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        str += +"|";
        out = str;
    }
    return out;
}
*/
QStringList DataAccessLayer::qsl_getbaseitemdata(const QString name, const QString type){
    //      0       1               2           3               4                   5               6           7
    //    name  |description    |short_desc |reference_book |reference_page    |price_value    |price_unit |rarity |
    //      8       9       10          11          12      13          14
    //    skill  |grip   |range_min  |range_max  |damage |deadliness | qualities
    //                          15                  16
    //    (qualities)| resistance_category | resist_value
    QStringList out;
    QSqlQuery query;
    if(type=="Weapon"){
    query.prepare("SELECT name_tr, description, short_desc, reference_book, reference_page, price_value, price_unit, rarity "
                  "from weapons where name_tr = ?");
    }
    else if(type == "Armor"){
     query.prepare("SELECT name_tr, description, short_desc, reference_book, reference_page, price_value, price_unit, rarity "
                  "from armor where name_tr = ?");
    }
    else {
     query.prepare("SELECT name_tr, description, short_desc, reference_book, reference_page, price_value, price_unit, rarity "
                  "from personal_effects where name_tr = ?");
    }

     query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
        out << query.value(1).toString();
        out << query.value(2).toString();
        out << query.value(3).toString();
        out << query.value(4).toString();
        out << query.value(5).toString();
        out << query.value(6).toString();
        out << query.value(7).toString();
    }
    return out;
}

QStringList DataAccessLayer::qsl_getitemqualities(const QString name, const QString type){
    QStringList out;
    QSqlQuery query;
    if(type=="Weapon"){
    query.prepare("SELECT quality_tr "
                  "from weapon_qualities where weapon_tr = ?");
    }
    else if(type == "Armor"){
     query.prepare("SELECT quality_tr "
                   "from armor_qualities where armor_tr = ?");
    }
    else {
        //TODO - handle other qualities
        return out;
    }

     query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
    }
    return out;
}

QList<QStringList> DataAccessLayer::ql_getweapondata(const QString name){
    QList<QStringList> out;
    QSqlQuery query;
    query.prepare("SELECT category_tr, skill_tr, grip_tr, range_min, range_max, damage, deadliness           "
                  "from weapons where name = ?                                                      ");

    query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        QStringList row;
        row << query.value(0).toString();
        row << query.value(1).toString();
        row << query.value(2).toString();
        row << query.value(3).toString();
        row << query.value(4).toString();
        row << query.value(5).toString();
        row << query.value(6).toString();
        out << row;

    }
    return out;
}

QList<QStringList> DataAccessLayer::ql_getarmordata(const QString name){
    QList<QStringList> out;
    QSqlQuery query;
    query.prepare("SELECT resistance_category_tr, resistance_value                                               "
                  "from armor_resistance where armor_tr = ?                                                      ");

    query.bindValue(0, name);
    query.exec();
    while (query.next()) {
        QStringList row;
        row << query.value(0).toString();
        row << query.value(1).toString();
        out << row;

    }
    return out;
}

QStringList DataAccessLayer::qsl_getschoolability(const QString school){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT school_ability_name_tr, reference_book, reference_page, school_ability_description FROM schools WHERE name_tr = ?");
    query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
        out << "School Ability";
        out << query.value(1).toString();
        out << query.value(2).toString();
        out << query.value(3).toString();
    }
    return out;
}

QStringList DataAccessLayer::qsl_getschoolmastery(const QString school){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT mastery_ability_name_tr, reference_book, reference_page, mastery_ability_description FROM schools WHERE name_tr = ?");
    query.bindValue(0, school);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
        out << "School Mastery";
        out << query.value(1).toString();
        out << query.value(2).toString();
        out << query.value(3).toString();
    }
    return out;
}

QStringList DataAccessLayer::qsl_gettitlemastery(const QString title){
    QStringList out;
    QSqlQuery query;
    query.prepare("SELECT title_ability_name_tr, reference_book, reference_page, title_ability_description FROM titles WHERE name_tr = ?");
    query.bindValue(0, title);
    query.exec();
    while (query.next()) {
        out << query.value(0).toString();
        out << title;
        out << query.value(1).toString();
        out << query.value(2).toString();
        out << query.value(3).toString();
    }
    return out;
}


QString DataAccessLayer::escapedCSV(QString unexc)
{
    if (!unexc.contains(QLatin1Char(',')))
        return unexc;
    return unexc.replace("\n","%0A"); //escape newlines
}

bool DataAccessLayer::tableToCsv(const QString filepath, const QString tablename, bool isDir) //DANGER - DO NOT ALLOW USERS TO CONTROL THIS
{
    QSqlQuery query;
    query.prepare("select * from "+tablename); //DANGER - DO NOT ALLOW USERS TO CONTROL THIS
    //QFile csvFile (filepath + "/" + tablename + ".csv");

    QFile csvFile;
    if(isDir){
        csvFile.setFileName(filepath+"/"+tablename+".csv");
    }
    else{
        csvFile.setFileName(filepath);
    }

    if (!csvFile.open(QFile::WriteOnly | QFile::Text)){
        qDebug("failed to open csv file");
        return false;
    }
    if (!query.exec()){
        qDebug("failed to run query");
        return false;
    }
    QTextStream outStream(&csvFile);
    outStream.setCodec("UTF-8");
    while (query.next()){
        const QSqlRecord record = query.record();
        for (int i=0, recCount = record.count() ; i<recCount ; ++i){
            if (i>0)
                outStream << ',';
            outStream << escapedCSV("\""+record.value(i).toString()+"\"");
        }
        outStream << '\n';
    }
    return true;
}

bool DataAccessLayer::queryToCsv(const QString querystr, QString filename) //DANGER - DO NOT ALLOW USERS TO CONTROL THIS
{
    QSqlQuery query;
    query.prepare(querystr); //DANGER - DO NOT ALLOW USERS TO CONTROL THIS
    //QFile csvFile (filepath + "/" + tablename + ".csv");

    QFile csvFile;
    csvFile.setFileName(filename);

    if (!csvFile.open(QFile::WriteOnly | QFile::Text)){
        qDebug("failed to open csv file");
        return false;
    }
    if (!query.exec()){
        qDebug("failed to run query");
        return false;
    }
    QTextStream outStream(&csvFile);
    outStream.setCodec("UTF-8");
    while (query.next()){
        const QSqlRecord record = query.record();
        for (int i=0, recCount = record.count() ; i<recCount ; ++i){
            if (i>0)
                outStream << ',';
            outStream << escapedCSV("\""+record.value(i).toString()+"\"");
        }
        outStream << '\n';
    }
    return true;
}

bool DataAccessLayer::exportTranslatableCSV(QString filename){
    QString q =
            "select strings.term, i18n.string_tr from (                                                                            "
            "select distinct * from (                                                                                   "
            "select distinct name as term from advantages_disadvantages                                                "
            "union select distinct ring as term from advantages_disadvantages                                   "
            "union select distinct types as term from advantages_disadvantages                                   "
            "union select distinct name as term from armor                                                                      "
            "union select distinct price_unit as term from armor                                                                      "
            "union select distinct quality as term from armor_qualities                                   "
            "union select distinct name as term from clans                                                                      "
            "union select distinct type as term from clans                                                                      "
            "union select distinct ring as term from clans                                                                      "
            "union select distinct skill as term from clans                                                                      "
            "union select distinct school as term from curriculum                                                                      "
            "union select distinct advance as term from curriculum                                                                      "
            "union select distinct clan as term from families                                                                      "
            "union select distinct name as term from families                                                                      "
            "union select distinct family as term from family_rings                                                                      "
            "union select distinct family as term from family_skills                                                                      "
            "union select distinct skill as term from family_skills                                                                      "
            "union select distinct ancestor as term from heritage_effects                                                                      "
            "union select distinct outcome as term from heritage_effects                                                                      "
            "union select distinct name as term from item_patterns                                                                      "
            "union select distinct quality as term from personal_effect_qualities                                                                      "
            "union select distinct price_unit as term from personal_effects                                                                      "
            "union select distinct quality as term from qualities                                                                      "
            "union select distinct name as term from rings                                                                      "
            "union select distinct outstanding_quality as term from rings                                                                      "
            "union select distinct ancestor as term from samurai_heritage                                                                      "
            "union select distinct effect_type as term from samurai_heritage                                                                      "
            "union select distinct effect_instructions as term from samurai_heritage                                                                      "
            "union select distinct school as term from school_rings                                                                      "
            "union select distinct ring as term from school_rings                                                                      "
            "union select distinct school as term from school_starting_outfit                                                                      "
            "union select distinct equipment as term from school_starting_outfit                                                                      "
            "union select distinct school as term from school_starting_skills                                                                      "
            "union select distinct school from school_starting_techniques                                                                      "
            "union select distinct technique from school_starting_techniques                                                                      "
            "union select distinct school from school_techniques_available                                                                      "
            "union select distinct technique from school_techniques_available                                                                      "
            "union select distinct name from schools                                                                      "
            "union select distinct role from schools                                                                      "
            "union select distinct clan from schools                                                                      "
            "union select distinct school_ability_name from schools                                                                      "
            "union select distinct mastery_ability_name from schools                                                                      "
            "union select distinct skill_group from skills                                                                      "
            "union select distinct skill from skills                                                                      "
            "union select distinct category from techniques                                                                      "
            "union select distinct subcategory from techniques                                                                      "
            "union select distinct name from techniques                                                                      "
            "union select distinct restriction from techniques                                                                      "
            "union select distinct title from title_advancements                                                                      "
            "union select distinct name from title_advancements                                                                      "
            "union select distinct type from title_advancements                                                                      "
            "union select distinct name from titles                                                                      "
            "union select distinct title_ability_name from titles                                                                      "
            "union select distinct weapon from weapon_qualities                                                                      "
            "union select distinct quality from weapon_qualities                                                                      "
            "union select distinct category from weapons                                                                      "
            "union select distinct name from weapons                                                                      "
            "union select distinct skill from weapons                                                                      "
            "union select distinct grip from weapons                                                                      "
            "union select distinct price_unit from weapons                                                                      "
            "union select distinct name from personal_effects                                                                       "
            ") where term is not NULL and term is not ''                                                                      "
            ") strings                                                                                                          "
            "left join i18n on strings.term = i18n.string                                                                        "
            ;


    return queryToCsv(q, filename);
}

//pulled from https://stackoverflow.com/questions/27318631/parsing-through-a-csv-file-in-qt
//which was in turn adapted from https://github.com/hnaohiro/qt-csv/blob/master/csv.cpp
QStringList DataAccessLayer::parseCSV(const QString &string)
{
    enum State {Normal, Quote} state = Normal;
    QStringList fields;
    QString value;

    for (int i = 0; i < string.size(); i++)
    {
        const QChar current = string.at(i);

        // Normal state
        if (state == Normal)
        {
            // Comma
            if (current == ',')
            {
                // Save field
                fields.append(value.trimmed());
                value.clear();
            }

            // Double-quote
            else if (current == '"')
            {
                state = Quote;
                value += current;
            }

            // Other character
            else
                value += current;
        }

        // In-quote state
        else if (state == Quote)
        {
            // Another double-quote
            if (current == '"')
            {
                if (i < string.size())
                {
                    // A double double-quote?
                    if (i+1 < string.size() && string.at(i+1) == '"')
                    {
                        value += '"';

                        // Skip a second quote character in a row
                        i++;
                    }
                    else
                    {
                        state = Normal;
                        value += '"';
                    }
                }
            }

            // Other character
            else
                value += current;
        }
    }

    if (!value.isEmpty())
        fields.append(value.trimmed());

    // Quotes are left in until here; so when fields are trimmed, only whitespace outside of
    // quotes is removed.  The quotes are removed here.
    for (int i=0; i<fields.size(); ++i)
        if (fields[i].length()>=1 && fields[i].left(1)=='"')
        {
            fields[i]=fields[i].mid(1);
            if (fields[i].length()>=1 && fields[i].right(1)=='"')
                fields[i]=fields[i].left(fields[i].length()-1);
        }

    return fields;
}

//NOTE - THIS ALLOWS DIRECT INJECTION OF DATA--BAD THINGS CAN HAPPEN.
//based on https://dustri.org/b/import-cvs-to-sqlite-with-qt.html
bool DataAccessLayer::importCSV(const QString filepath, const QString tablename, bool isDir){
    bool success = true;
    QFile f;
    if(isDir){
        f.setFileName(filepath+"/"+tablename+".csv");
    }
    else{
        f.setFileName(filepath);
    }
    //QFile f(filepath+"/"+tablename+".csv");
    if(f.open (QIODevice::ReadOnly)){
        QSqlQuery query;
        success &= query.exec("DELETE FROM "+tablename);
        if(!success) {
            qDebug()<< "Could not delete "+tablename;
        }
        QTextStream ts (&f);

        while(!ts.atEnd()){

            QString req = "INSERT INTO "+tablename+" VALUES(";
            // split every lines on comma
            const QStringList line = parseCSV(ts.readLine());
            /*for every values on a line,
                append it to the INSERT request*/
            for(int i=0; i<line .length ();++i){
                req.append("\""+line.at(i)+"\""); //needed quotes to get SQLITE to recognize it
                req.append(",");
            }
            req.chop(1); // remove the trailing comma
            req.append(");"); // close the "VALUES([...]" with a ");"
            const bool isuccess = query.exec(req.replace("%0A","\n")); //fix the encoded %0A
            if(!isuccess) {
                qDebug() << "Could not insert using "+ req;
            }
            success &= isuccess;
        }
        f.close ();
    }
    else { //couldn't open file
        return !success;
    }
    return success;
}
