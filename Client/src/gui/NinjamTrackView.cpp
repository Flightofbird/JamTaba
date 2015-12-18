#include "NinjamTrackView.h"
#include "ui_TrackGroupView.h"
#include "BaseTrackView.h"
#include "ui_BaseTrackView.h"
#include <QLineEdit>
#include <QLabel>
#include <QDebug>
#include <QStyle>
#include "MainController.h"
#include "Utils.h"

//+++++++++++++++++++++++++
NinjamTrackView::NinjamTrackView(Controller::MainController *mainController, long trackID, QString channelName, Persistence::CacheEntry initialValues)
    :BaseTrackView(mainController, trackID), cacheEntry(initialValues)
{
    channelNameLabel = new QLabel();
    channelNameLabel->setObjectName("channelName");
    channelNameLabel->setText(channelName);

    ui->mainLayout->insertSpacing(0, 12);
    ui->mainLayout->insertWidget(1, channelNameLabel);

    chunksDisplay = new IntervalChunksDisplay(this);
    ui->mainLayout->addSpacing(6);
    ui->mainLayout->addWidget(chunksDisplay);

    setUnlightStatus(true);//disabled/grayed until receive the first bytes. When the first bytes
    //are downloaded the 'on_channelXmitChanged' slot is executed and this track is enabled.

    //remember last track values
    ui->levelSlider->setValue(initialValues.getGain() * 100);
    ui->panSlider->setValue(initialValues.getPan() * ui->panSlider->maximum());
    if(initialValues.isMuted()){
        ui->muteButton->click();
    }
    if(initialValues.getBoost() < 1){
        ui->buttonBoostMinus12->click();
    }
    else{
        if(initialValues.getBoost() > 1){
            ui->buttonBoostPlus12->click();
        }
        else{
            ui->buttonBoostZero->click();
        }
    }

}

//+++++= interval chunks visual feedback ++++++=
void NinjamTrackView::finishCurrentDownload(){
    this->chunksDisplay->pushNewDownloadInStack();
}

void NinjamTrackView::removeFirstDownloadInStack(){
    this->chunksDisplay->popDownloadFromStack();
}

void NinjamTrackView::incrementDownloadedChunks(){
    this->chunksDisplay->incrementDownloadedChunks();
}

void NinjamTrackView::resetDownloadedChunks(){
    this->chunksDisplay->reset();
}

//+++++++++++++++++++

void NinjamTrackView::setChannelName(QString name){
    this->channelNameLabel->setText(name);
    int nameWidth = this->channelNameLabel->fontMetrics().width(name);
    if(nameWidth <= this->channelNameLabel->contentsRect().width()){
        this->channelNameLabel->setAlignment(Qt::AlignCenter);
    }
    else{
        this->channelNameLabel->setAlignment(Qt::AlignLeft);
    }
    this->channelNameLabel->setToolTip(name);
}

//+++++++++++++++++++++

void NinjamTrackView::onPanSliderMoved(int value){
    BaseTrackView::onPanSliderMoved(value);
    cacheEntry.setPan(mainController->getTrackNode(getTrackID())->getPan());
    mainController->getUsersDataCache()->updateUserCacheEntry(cacheEntry);
}

void NinjamTrackView::onFaderMoved(int value){
    BaseTrackView::onFaderMoved(value);
    cacheEntry.setGain( value/100.0 );
    mainController->getUsersDataCache()->updateUserCacheEntry(cacheEntry);
}

void NinjamTrackView::onMuteClicked(){
    BaseTrackView::onMuteClicked();
    cacheEntry.setMuted(mainController->getTrackNode(getTrackID())->isMuted());
    mainController->getUsersDataCache()->updateUserCacheEntry(cacheEntry);
}

void NinjamTrackView::onBoostButtonClicked(){
    BaseTrackView::onBoostButtonClicked();
    cacheEntry.setBoost(mainController->getTrackNode(getTrackID())->getBoost());
    mainController->getUsersDataCache()->updateUserCacheEntry(cacheEntry);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void NinjamTrackGroupView::updateGeoLocation(){
    Geo::Location location = mainController->getGeoLocation(this->userIP);
    countryLabel->setText("<img src=:/flags/flags/" + location.getCountryCode().toLower() +".png> <br>" + location.getCountryName());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

NinjamTrackGroupView::NinjamTrackGroupView(QWidget *parent, Controller::MainController *mainController, long trackID, QString channelName, Persistence::CacheEntry initialValues)
    :TrackGroupView(parent), mainController(mainController), userIP(initialValues.getUserIP())
{
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred));

    //change the top panel layout to vertical (original is horizontal)
    ui->topPanel->layout()->removeWidget(ui->groupNameField);
    delete ui->topPanel->layout();
    ui->topPanel->setLayout(new QVBoxLayout());
    ui->topPanel->layout()->addWidget(ui->groupNameField);

    setGroupName(initialValues.getUserName());

    //country flag label
    countryLabel = new QLabel();
    countryLabel->setObjectName("countryLabel");
    countryLabel->setTextFormat(Qt::RichText);
    updateGeoLocation();
    //countryLabel->setText("<img src=:/flags/flags/" + countryCode +".png> <br>" + countryName);

    ui->topPanel->layout()->addWidget(countryLabel);

    //create the first subchannel by default
    addTrackView(new NinjamTrackView(mainController, trackID, channelName, initialValues));

    ui->groupNameField->setReadOnly(true);

}

void NinjamTrackGroupView::popFullyDownloadedIntervals(){
    foreach (BaseTrackView* trackView, trackViews) {
        dynamic_cast<NinjamTrackView*>(trackView)->removeFirstDownloadInStack();
    }
}

void NinjamTrackGroupView::resetDownloadedIntervals(){
    foreach (BaseTrackView* trackView, trackViews) {
        dynamic_cast<NinjamTrackView*>(trackView)->resetDownloadedChunks();
    }
}

void NinjamTrackGroupView::setNarrowStatus(bool narrow){
    foreach (BaseTrackView* trackView, trackViews) {
        bool setToWide = !narrow && trackViews.size() <= 1;
        if(setToWide){
           trackView->setToWide();
        }
        else{

           trackView->setToNarrow();
        }
    }
}

NinjamTrackGroupView::~NinjamTrackGroupView()
{

}
