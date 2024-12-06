#pragma once
#include <qwidget.h>
#include <qevent.h>
#include <qlabel.h>
#include <qpropertyanimation.h>

class toolBar :
	public QWidget
{
public:
	toolBar(QWidget* parent);
	~toolBar() {}
	void resize(int x,int y) { resize(QSize(x , y)); }
	void resize(QSize size) {
		geometryRect.setSize(size);
		aniEndSize = size;
		QWidget::resize(size);
	}
	void move(int ax, int ay) { move(QPoint(ax, ay)); }
	void move(QPoint pos) {
		if (previousMousePos == QPoint()) {
			geometryRect.setTopLeft(pos);
			if(isCustomized())
				setBackUpWindowRect(parentWidget()->geometry());
			QWidget::move(pos);
		}
	}
	void restoreSize() {
		if(backup.isValid())
			QWidget::resize(backup);
		backup = QSize();
	}
	void backupSize() {
		backup = size();
	}
	bool isCustomized() const {//判断是否自定义了工具栏位置
		return Customized;
	}
	void setCustomized(bool j) {
		Customized = j;
	}
	QSize getAniEndSize() const {
		return aniEndSize;
	}
	QSize size() const {
		return aniEndSize;
	}
	QRect geometry() const {
		return geometryRect;
	}
	QRect getBackUpWindowRect() const {
		return bkWindowRect;
	}
	void setBackUpWindowRect(QRect r) {
		bkWindowRect = r;
	}
protected:
	void mousePressEvent(QMouseEvent* e);
	void mouseReleaseEvent(QMouseEvent* e);
	void mouseMoveEvent(QMouseEvent* e);
	void paintEvent(QPaintEvent* e);
private:
	QPoint previousMousePos;
	QPoint substractPos;//鼠标-控件
	QSize backup;
	QSize aniEndSize = QWidget::size();
	QRect geometryRect = QWidget::geometry();
	//bool animation = true;
	QPropertyAnimation* FoldAni;
	bool Customized = false;
	QRect bkWindowRect;
};

