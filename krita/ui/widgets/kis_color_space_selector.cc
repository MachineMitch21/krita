    /*
 *  Copyright (C) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_space_selector.h"

#include <KoColorProfile.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoID.h>

#include <QDesktopServices>

#include <kcomponentdata.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <kio/copyjob.h>

#include "kis_factory2.h"

#include "ui_wdgcolorspaceselector.h"

struct KisColorSpaceSelector::Private {
    Ui_WdgColorSpaceSelector* colorSpaceSelector;
};

KisColorSpaceSelector::KisColorSpaceSelector(QWidget* parent) : QWidget(parent), d(new Private)
{
    setObjectName("KisColorSpaceSelector");
    d->colorSpaceSelector = new Ui_WdgColorSpaceSelector;
    d->colorSpaceSelector->setupUi(this);
    d->colorSpaceSelector->cmbColorModels->setIDList(KoColorSpaceRegistry::instance()->colorModelsList(KoColorSpaceRegistry::OnlyUserVisible));
    fillCmbDepths(d->colorSpaceSelector->cmbColorModels->currentItem());
    connect(d->colorSpaceSelector->cmbColorModels, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbDepths(const KoID &)));
    connect(d->colorSpaceSelector->cmbColorDepth, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbProfiles()));
    connect(d->colorSpaceSelector->cmbColorModels, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbProfiles()));
    connect(d->colorSpaceSelector->cmbProfile, SIGNAL(activated(const QString &)),
            this, SLOT(colorSpaceChanged()));
    connect(d->colorSpaceSelector->bnInstallProfile, SIGNAL(clicked()), this, SLOT(installProfile()));

    fillCmbProfiles();
}

KisColorSpaceSelector::~KisColorSpaceSelector()
{
    delete d->colorSpaceSelector;
    delete d;
}


void KisColorSpaceSelector::fillCmbProfiles()
{
    QString s = KoColorSpaceRegistry::instance()->colorSpaceId(d->colorSpaceSelector->cmbColorModels->currentItem(), d->colorSpaceSelector->cmbColorDepth->currentItem());
    d->colorSpaceSelector->cmbProfile->clear();

    const KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->colorSpaceFactory(s);
    if (csf == 0) return;

    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);

    foreach(const KoColorProfile *profile, profileList) {
        d->colorSpaceSelector->cmbProfile->addSqueezedItem(profile->name());
    }
    d->colorSpaceSelector->cmbProfile->setCurrent(csf->defaultProfile());
    colorSpaceChanged();
}

void KisColorSpaceSelector::fillCmbDepths(const KoID& id)
{
    KoID activeDepth = d->colorSpaceSelector->cmbColorDepth->currentItem();
    d->colorSpaceSelector->cmbColorDepth->clear();
    QList<KoID> depths = KoColorSpaceRegistry::instance()->colorDepthList(id, KoColorSpaceRegistry::OnlyUserVisible);
    d->colorSpaceSelector->cmbColorDepth->setIDList(depths);
    if (depths.contains(activeDepth)) {
        d->colorSpaceSelector->cmbColorDepth->setCurrent(activeDepth);
    }
}

const KoColorSpace* KisColorSpaceSelector::currentColorSpace()
{
    return KoColorSpaceRegistry::instance()->colorSpace(
               d->colorSpaceSelector->cmbColorModels->currentItem().id(), d->colorSpaceSelector->cmbColorDepth->currentItem().id()
               , d->colorSpaceSelector->cmbProfile->itemHighlighted());
}

void KisColorSpaceSelector::setCurrentColorModel(const KoID& id)
{
    d->colorSpaceSelector->cmbColorModels->setCurrent(id);
    fillCmbDepths(id);
}

void KisColorSpaceSelector::setCurrentColorDepth(const KoID& id)
{
    d->colorSpaceSelector->cmbColorDepth->setCurrent(id);
    fillCmbProfiles();
}

void KisColorSpaceSelector::setCurrentProfile(const QString& name)
{
    d->colorSpaceSelector->cmbProfile->setCurrent(name);
}

void KisColorSpaceSelector::setCurrentColorSpace(const KoColorSpace* colorSpace)
{
  setCurrentColorModel(colorSpace->colorModelId());
  setCurrentColorDepth(colorSpace->colorDepthId());
  setCurrentProfile(colorSpace->profile()->name());
}

void KisColorSpaceSelector::colorSpaceChanged()
{
    bool valid = d->colorSpaceSelector->cmbProfile->count() != 0;
    emit(selectionChanged(valid));
    if(valid) {
        emit colorSpaceChanged(currentColorSpace());
    }
}

void KisColorSpaceSelector::installProfile()
{
    KUrl homedir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    QStringList profileNames = KFileDialog::getOpenFileNames(homedir, "*.icm *.icc", this, "Install Color Profiles");

    KoColorSpaceEngine *iccEngine = KoColorSpaceEngineRegistry::instance()->get("icc");
    Q_ASSERT(iccEngine);

    QString saveLocation = KGlobal::mainComponent().dirs()->saveLocation("icc_profiles");

    foreach (QString profileName, profileNames) {
        KIO::copy(KUrl(profileName), saveLocation);
        iccEngine->addProfile(saveLocation + profileName);
    }

    fillCmbProfiles();
}

#include "kis_color_space_selector.moc"
