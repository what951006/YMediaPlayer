#include "TEST.h"
#include "DerSlider.h"
#include "AboutUs.h"

#include "YMediaPlayer.h"

#include <QTime>
#include <QDebug>
#include <QFileDialog>
#include <QPainter>
#include <windows.h>


TEST::TEST(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.le_media_url->setPlaceholderText("Please Input URL Here~");

	QObject::connect(this, &TEST::sig_Update, this, [&](void *data, int w, int h) {
		if (w <= 0 || h <= 0 || !data)
			return;
		img_ = QImage((uchar*)data, w, h, QImage::Format_RGB32).copy();
		update(rect());
	}, Qt::BlockingQueuedConnection);

	QObject::connect(this, &TEST::sig_Dur, this, [&](int dur) {
		QTime tim(dur / 3600, dur / 60, dur % 60, 0);
		QString str = tim.toString("hh:mm:ss");
		ui.lab_dur->setText(str);
		ui.slider_media->setMaximum(dur);
		ui.slider_media->setMinimum(0);
		ui.slider_media->setValue(0);
	}, Qt::QueuedConnection);


	QObject::connect(this, &TEST::sig_Pos, this, [&](int curpos) {
		QTime tim(curpos / 3600, curpos / 60, curpos % 60, 0);
		QString str = tim.toString("hh:mm:ss");
		ui.lab_cur->setText(str);
		ui.slider_media->setValue(curpos);
	}, Qt::QueuedConnection);

	QObject::connect(this, &TEST::sig_Buffer, this, [&](float percent) {
		ui.slider_media->SetBufferPercent(percent);
	}, Qt::QueuedConnection);

	QObject::connect(ui.slider_media, &DerSlider::sig_Clicked, this, [&](float value)
	{
		player_->Seek(value);
	});

	QObject::connect(ui.btn_play, &QPushButton::clicked, this, [&] {
		if (player_->IsPlaying())
			player_->Pause();
		else
			player_->Play();
	});

	QObject::connect(ui.btn_support, &QPushButton::clicked, this, [&] {
		AboutUs us(this);
		us.exec();
	});



	QObject::connect(ui.btn_open, &QPushButton::clicked, this, [&] {

		QString &&fileName = ui.le_media_url->text();
		if (fileName.isEmpty())
			return;
		player_->SetMediaFromFile(fileName.toStdString().c_str());
		player_->Play();
	});

	player_ = CreatePlayer(MODE_WIN_WAV, MODE_USER, this);

	player_->SetDurationChangedFunction([](void*opa, int dur) {
		TEST *widget = (TEST*)opa;
		emit widget->sig_Dur(dur);
	});
	player_->SetCurrentChangedFucnton([](void*opa, int cur) {
		TEST *widget = (TEST*)opa;
		emit widget->sig_Pos(cur);
	});

	player_->SetUserHandleVideoFunction([](void *opa, void*data, int width, int height) {
		TEST *widget = (TEST*)opa;
		emit widget->sig_Update(data, width, height);
		//Sleep(100);
	});

	player_->SetBufferFunction([](void *opaque, float percent){
		TEST *widget = (TEST*)opaque;
		emit widget->sig_Buffer(percent);
	});

	//http://hc.yinyuetai.com/uploads/videos/common/02D30168547A579F07E92D27B0DA34D0.mp4?sc=068dd881b47be705
	ui.le_media_url->setText("http://hc.yinyuetai.com/uploads/videos/common/02D30168547A579F07E92D27B0DA34D0.mp4?sc=068dd881b47be705");
}

TEST::~TEST()
{
	DeletePlayer(player_);
}

void TEST::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.drawImage(0, 0, img_);
}
