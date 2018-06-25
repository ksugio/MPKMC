#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Created on Thu Jan 28 12:04:57 2016

@author: Kenjiro Sugio
"""

import MPKMC
import MPGLKMC
import sys
import math
import numpy as np
from PyQt4 import QtCore, QtGui, QtOpenGL
from OpenGL import GL
from PIL import Image
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

"""
GLWidget Class
"""
class GLWidget(QtOpenGL.QGLWidget):
  def __init__(self, parent=None):
    QtOpenGL.QGLWidget.__init__(self, QtOpenGL.QGLFormat(QtOpenGL.QGL.SampleBuffers), parent)
    self.lastPos = QtCore.QPoint()
    self.mouseMode = 0
    self.dispMode = 0
    self.kmc = None
    self.draw = MPGLKMC.draw()
    self.scene = MPGLKMC.scene()
    self.scene.light_add(1.0, 1.0, 1.0, 0.0)
    self.scene.proj = 0    
    self.model = [MPGLKMC.model(), MPGLKMC.model()]
    self.cmp = MPGLKMC.colormap()
    self.tabid = 0
    self.__width = 800
    self.__height = 600    

  def minimumSizeHint(self):
    return QtCore.QSize(320, 240)

  def sizeHint(self):
    return QtCore.QSize(800, 600)

  def initializeGL(self):
    self.scene.setup()

  def paintGL(self):
    GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT)
    if self.kmc:
      if self.dispMode == 0:
        GL.glPushMatrix()
        self.model[0].transform()
        self.draw.transform(self.kmc)
        self.draw.atoms(self.kmc, self.cmp)
        self.draw.frame(self.kmc) 
        GL.glTranslate(-0.3, -0.3, -0.3)
        self.draw.axis(self.kmc, self.kmc.size, 0.1)
        GL.glPopMatrix()
        self.drawColormap()
        stx = (2.0 * 10 - self.__width) / self.__height
        sty = 2.0*(self.__height - 20) / self.__height - 1.0
        self.drawString(stx, sty, str(self.kmc.step) + ' step')
      elif self.dispMode == 1 and self.kmc.ntable > 0:
        GL.glPushMatrix()
        self.model[1].transform()
        self.draw.transform(self.kmc)
        self.draw.cluster(self.kmc, self.cmp, self.tabid)     
        GL.glTranslate(-1.5, -1.5, -1.5)
        self.draw.axis(self.kmc, (3, 3, 3), 0.1)
        GL.glPopMatrix()
        self.drawColormap()
        stx = (2.0 * 10 - self.__width) / self.__height
        sty = 2.0*(self.__height - 20) / self.__height - 1.0
        item = self.kmc.table_item(self.tabid)
        s = 'ID %d,   Energy %f,   Count %d' % (self.tabid, item[1], item[2])
        self.drawString(stx, sty, s)

  def drawColormap(self):
    GL.glPushMatrix()
    GL.glRotated(90.0, 0.0, 0.0, 1.0)
    GL.glRotated(90.0, 1.0, 0.0, 0.0)
    GL.glTranslate((2.0 - self.__width) / self.__height, -self.cmp.size[1]/2.0, self.scene.znear - 1.0e-6)
    self.cmp.draw()
    GL.glPopMatrix()

  def resizeGL(self, width, height):
    self.__width = width
    self.__height = height
    self.scene.resize(width, height) 

  def mousePressEvent(self, event):
    self.lastPos = QtCore.QPoint(event.pos())    

  def mouseReleaseEvent(self, event):
    self.model[self.dispMode].inverse()

  def mouseMoveEvent(self, event):
    dx = event.x() - self.lastPos.x()
    dy = event.y() - self.lastPos.y()
    mod = QtGui.QApplication.keyboardModifiers()
    if event.buttons() & QtCore.Qt.LeftButton:
        if self.mouseMode == 0:
            if mod == QtCore.Qt.ControlModifier:
                cx = self.__width / 2
                cy = self.__height / 2
                if event.x() <= cx and event.y() <= cy:
                    ax = math.pi * (-dx + dy) / self.__height
                elif event.x() > cx and event.y() <= cy:
                    ax = math.pi * (-dx - dy) / self.__height
                elif event.x() <= cx and event.y() > cy:
                    ax = math.pi * (dx + dy) / self.__height
                elif event.x() > cx and event.y() > cy:
                    ax = math.pi * (dx - dy) / self.__height
                self.model[self.dispMode].rot_x(-ax)
            else:
                az = math.pi * dx / self.__height
                self.model[self.dispMode].rot_z(-az)
                ay = math.pi *dy / self.__height
                self.model[self.dispMode].rot_y(-ay)
        elif self.mouseMode == 1:
            if mod == QtCore.Qt.ControlModifier:
                mx = 2.0 * dy / self.__height
                self.model[self.dispMode].trans_x(-mx)
            else:
                my = 2.0 * dx / self.__height
                self.model[self.dispMode].trans_y(my)
                mz = 2.0 * dy / self.__height
                self.model[self.dispMode].trans_z(-mz)
        elif self.mouseMode == 2:
            s = 1.0 - float(dy) / self.__height
            self.model[self.dispMode].zoom(s)
        self.updateGL()
        self.lastPos = QtCore.QPoint(event.pos())

  def initModel(self, mid):
    self.model[mid].init()

  def fitModel(self, mid):
    if mid == 0:
      region = self.draw.atoms_region(self.kmc)
    elif mid == 1:
      region = self.draw.cluster_region(self.kmc)
    self.model[mid].fit_center(region)
    aspect = float(self.__width)/float(self.__height)
    self.model[mid].fit_scale(region, aspect)

  def cmpRange(self):
    self.draw.colormap_range(self.kmc, self.cmp)

  def screenShot(self):
    screenshot = GL.glReadPixels(0, 0, self.width(), self.height(), GL.GL_RGBA, GL.GL_UNSIGNED_BYTE)
    img = Image.frombuffer("RGBA", (self.width(), self.height()), screenshot, "raw", "RGBA", 0, 0)
    return img

  def drawString(self, x, y, s):
    GL.glPushAttrib(GL.GL_LIGHTING_BIT)
    GL.glDisable(GL.GL_LIGHTING)
    GL.glRasterPos3d(self.scene.znear - 1.0e-6, x, y)
    GL.glColor3fv(self.cmp.font_color)
    MPGLKMC.text_bitmap(s, self.cmp.font_type)
    GL.glPopAttrib()

  def stepFirst(self):
    if self.kmc:
      if self.dispMode == 0:
        self.kmc.step_go(0)
      elif self.dispMode == 1:
        self.tabid = 0
      self.updateGL()
      
  def stepBackward(self):
    if self.kmc:
      if self.dispMode == 0:
        self.kmc.step_backward(1)
      elif self.dispMode == 1 and self.tabid > 0:
        self.tabid = self.tabid - 1
      self.updateGL() 
    
  def stepForward(self):
    if self.kmc:
      if self.dispMode == 0:
        self.kmc.step_forward(1)
      elif self.dispMode == 1 and self.tabid < self.kmc.ntable - 1 :
        self.tabid = self.tabid + 1
      self.updateGL()

  def stepLast(self):
    if self.kmc:
      if self.dispMode == 0:
        self.kmc.step_go(self.kmc.nevent)
      elif self.dispMode == 1:
        self.tabid = self.kmc.ntable - 1
      self.updateGL()

"""
StepGoDialog
""" 
class StepGoDialog(QtGui.QDialog):
  def __init__(self, parent, kmc):
    QtGui.QDialog.__init__(self, parent)
    self.kmc = kmc
    self.setWindowTitle("Step Go")
    vbox = QtGui.QVBoxLayout(self) 
    self.spin = QtGui.QSpinBox()
    self.spin.setMinimum(0)
    self.spin.setMaximum(kmc.nevent)
    self.spin.setValue(kmc.step)
    vbox.addWidget(self.spin)
    self.buttonb = QtGui.QDialogButtonBox()
    vbox.addWidget(self.buttonb)
    self.buttonb.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
    self.buttonb.accepted.connect(self.Accept)
    self.buttonb.rejected.connect(self.reject)

  def Accept(self):
    step = self.spin.value()
    self.kmc.step_go(step)
    self.accept()

"""
SetDispDialog
""" 
class SetDispDialog(QtGui.QDialog):
  def __init__(self, parent, kmc, draw):
    QtGui.QDialog.__init__(self, parent)
    self.draw = draw
    self.setWindowTitle("Set Display")
    vbox = QtGui.QVBoxLayout(self)
    self.table = QtGui.QTableWidget(draw.ntypes, 3)
    self.table.setHorizontalHeaderLabels(["Type", "Display", "Diameter"])
    for i in range(draw.ntypes):
      self.table.setItem(i, 0, QtGui.QTableWidgetItem(str(draw.types(i))))
      item = QtGui.QTableWidgetItem()
      if draw.get_disp(i) == 0:
        item.setCheckState(QtCore.Qt.Unchecked)
      else:
        item.setCheckState(QtCore.Qt.Checked)
      self.table.setItem(i, 1, item)
      self.table.setItem(i, 2, QtGui.QTableWidgetItem(str(draw.get_dia(i))))
    vbox.addWidget(self.table)
    self.buttonb = QtGui.QDialogButtonBox()
    vbox.addWidget(self.buttonb)
    self.buttonb.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
    self.buttonb.accepted.connect(self.Accept)
    self.buttonb.rejected.connect(self.reject)

  def Accept(self):
    for row in range(self.table.rowCount()):
      item = self.table.item(row, 1)
      if item.checkState() == QtCore.Qt.Unchecked:
        self.draw.set_disp(row, 0)
      else:
        self.draw.set_disp(row, 1)
      dia = float(self.table.item(row, 2).text())
      self.draw.set_dia(row, dia)
    self.accept()

"""
SetShiftDialog
""" 
class SetShiftDialog(QtGui.QDialog):
  def __init__(self, parent, kmc, draw):
    QtGui.QDialog.__init__(self, parent)
    self.draw = draw
    self.setWindowTitle("Set Shift")
    vbox = QtGui.QVBoxLayout(self)
    self.spin_x = QtGui.QSpinBox()
    self.spin_x.setValue(draw.shift[0])
    self.spin_x.setMaximum(kmc.size[0])
    self.spin_x.setMinimum(-kmc.size[0])
    vbox.addWidget(self.spin_x)
    self.spin_y = QtGui.QSpinBox()
    self.spin_y.setValue(draw.shift[1])
    self.spin_y.setMaximum(kmc.size[1])
    self.spin_y.setMinimum(-kmc.size[1])
    vbox.addWidget(self.spin_y)
    self.spin_z = QtGui.QSpinBox()
    self.spin_z.setValue(draw.shift[2])
    self.spin_z.setMaximum(kmc.size[2])
    self.spin_z.setMinimum(-kmc.size[2])
    vbox.addWidget(self.spin_z)
    self.buttonb = QtGui.QDialogButtonBox()
    vbox.addWidget(self.buttonb)
    self.buttonb.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
    self.buttonb.accepted.connect(self.Accept)
    self.buttonb.rejected.connect(self.reject)

  def Accept(self):
    x = self.spin_x.value()
    y = self.spin_y.value()
    z = self.spin_z.value()
    self.draw.shift = (x, y, z)
    self.accept()

"""
EnergyHistoryDialog
""" 
class EnergyHistoryCanvas(FigureCanvas):
  def __init__(self, parent=None, width=8, height=6, dpi=72):
    self.fig = Figure(figsize=(width, height), dpi=dpi)
    self.axes = self.fig.add_subplot(111)
    self.axes.hold(False)
    FigureCanvas.__init__(self, self.fig)
    self.setParent(parent)
    FigureCanvas.setSizePolicy(self, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
    FigureCanvas.updateGeometry(self)

  def drawGraph(self, ehist):
    self.axes.cla()
    st = np.arange(ehist.shape[0])
    self.axes.plot(st, ehist, 'k-')
    self.axes.set_xlabel('Event Step')
    self.axes.set_ylabel('Total Energy')
    self.draw()

  def saveGraph(self, fname):
    self.fig.savefig(fname)

class EnergyHistoryDialog(QtGui.QDialog):
  def __init__(self, parent, kmc):
    QtGui.QDialog.__init__(self, parent)
    self.ehist = np.zeros(kmc.nevent+1, dtype=np.float)
    kmc.energy_history(self.ehist)
    self.setWindowTitle("Energy History")
    vbox = QtGui.QVBoxLayout(self)
    self.canvas = EnergyHistoryCanvas()
    vbox.addWidget(self.canvas)
    hbox1 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox1)
    button1 = QtGui.QPushButton("Save Fig")
    button1.clicked.connect(self.saveFig)
    hbox1.addWidget(button1)
    button2 = QtGui.QPushButton("Close")
    button2.clicked.connect(self.reject)
    hbox1.addWidget(button2)
    self.canvas.drawGraph(self.ehist)

  def saveFig(self):
    fname = QtGui.QFileDialog.getSaveFileName(self, 'Save Fig')
    if fname:
      self.canvas.saveGraph(str(fname))    

"""
MainWindow
"""    
class MainWindow(QtGui.QMainWindow):
  def __init__(self, parent=None):
    QtGui.QWidget.__init__(self, parent)
    self.setWindowTitle(self.tr("MPKMCViewer"))
    self.glwidget = GLWidget()
    self.setCentralWidget(self.glwidget)
    self.MainMenuBar()
    self.MainToolBar()
    self.compress = 8

  def MainMenuBar(self):
    menubar = QtGui.QMenuBar(self)
    self.setMenuBar(menubar)
    file_menu = QtGui.QMenu('File', self)
    file_menu.addAction('Open', self.fileOpen)
    file_menu.addAction('Save', self.fileSave)
    file_menu.addAction('Save Image', self.fileSaveImage)
    file_menu.addAction('Quit', QtCore.QCoreApplication.instance().quit)   
    menubar.addMenu(file_menu)
    view_menu = QtGui.QMenu('View', self)
    view_menu.addAction('Set Display', self.setDispDialog)
    view_menu.addAction('Set Shift', self.setShiftDialog)
    view_menu.addAction('Energy History', self.energyHistoryDialog)
    menubar.addMenu(view_menu)

  def fileOpen(self):
    fname = QtGui.QFileDialog.getOpenFileName(self, 'Open file')
    if fname:
      self.glwidget.kmc = MPKMC.read(str(fname))
      self.glwidget.kmc.total_energy(None)
      self.glwidget.initModel(0)
      self.glwidget.fitModel(0)
      self.glwidget.initModel(1)
      self.glwidget.fitModel(1)      
      self.glwidget.cmpRange()
      self.glwidget.updateGL()

  def fileSave(self):
    if self.glwidget.kmc:
      fname = QtGui.QFileDialog.getSaveFileName(self, 'Save file')
      if fname:
        self.glwidget.kmc.write(str(fname), self.compress)
        
  def fileSaveImage(self):
    if self.glwidget.kmc:
      fname = QtGui.QFileDialog.getSaveFileName(self, 'Save image')
      if fname:      
        img = self.glwidget.screenShot()
        img.save(str(fname))

  def addSoluteDialog(self):
    if self.glwidget.kmc:
      dlg = AddSoluteDialog(self, self.glwidget.kmc)
      dlg.exec_()

  def addSoluteRandomDialog(self):
    if self.glwidget.kmc:
      dlg = AddSoluteRandomDialog(self, self.glwidget.kmc)
      dlg.exec_()

  def setDispDialog(self):
    if self.glwidget.kmc:
      dlg = SetDispDialog(self, self.glwidget.kmc, self.glwidget.draw)
      dlg.exec_()

  def setShiftDialog(self):
    if self.glwidget.kmc:
      dlg = SetShiftDialog(self, self.glwidget.kmc, self.glwidget.draw)
      dlg.exec_()

  def energyHistoryDialog(self):
    if self.glwidget.kmc:
      dlg = EnergyHistoryDialog(self, self.glwidget.kmc)
      dlg.exec_()
      
  def MainToolBar(self):
    toolbar = QtGui.QToolBar(self)
    toolbar.addAction('Init', self.initModel)
    toolbar.addAction('Fit', self.fitModel)    
    toolbar.addSeparator()    
    group1 = QtGui.QButtonGroup(self)
    button1 = ToolButton('Rot', self.setMouseMode)
    button1.toggle()
    group1.addButton(button1)
    toolbar.addWidget(button1)
    button2 = ToolButton('Trans', self.setMouseMode)
    group1.addButton(button2)
    toolbar.addWidget(button2)
    button3 = ToolButton('Zoom', self.setMouseMode)
    group1.addButton(button3)
    toolbar.addWidget(button3)
    toolbar.addSeparator()
    group2 = QtGui.QButtonGroup(self)
    button4 = ToolButton('Type', self.setDrawKind)
    button4.toggle()
    group2.addButton(button4)
    toolbar.addWidget(button4)
    button5 = ToolButton('Energy', self.setDrawKind)
    group2.addButton(button5)
    toolbar.addWidget(button5)
    button6 = ToolButton('Cluster', self.setDrawKind)
    group2.addButton(button6)
    toolbar.addWidget(button6)
    toolbar.addSeparator()
    toolbar.addAction('<<', self.glwidget.stepFirst)    
    toolbar.addAction('<', self.glwidget.stepBackward)
    self.playb = ToolButton('Play', self.StepPlay)    
    toolbar.addWidget(self.playb)
    toolbar.addAction('>', self.glwidget.stepForward)
    toolbar.addAction('>>', self.glwidget.stepLast) 
    toolbar.addAction('Go', self.StepGo)
    self.addToolBar(toolbar)

  def setMouseMode(self, pressed):
    txt = self.sender().text()
    if txt == "Rot":
        self.glwidget.mouseMode = 0
    elif txt == "Trans":
        self.glwidget.mouseMode = 1 
    elif txt == "Zoom":
        self.glwidget.mouseMode = 2

  def setDrawKind(self, pressed):
    txt = self.sender().text()
    if txt == "Type":
      self.glwidget.dispMode = 0
      self.glwidget.draw.kind = 0
    elif txt == "Energy":
      self.glwidget.dispMode = 0
      self.glwidget.draw.kind = 1
    elif txt == "Cluster":
      self.glwidget.dispMode = 1
    self.glwidget.updateGL()

  def initModel(self):
    self.glwidget.initModel(self.glwidget.dispMode)
    self.glwidget.updateGL()

  def fitModel(self):
    self.glwidget.fitModel(self.glwidget.dispMode)
    self.glwidget.updateGL()
      
  def StepGo(self):
    if self.glwidget.kmc:
      dlg = StepGoDialog(self, self.glwidget.kmc)
      dlg.exec_()    

  def StepPlay(self):
    if self.glwidget.dispMode == 0:
      if self.playb.isChecked():
        self.timer = QtCore.QTimer()
        self.timer.setInterval(10)
        self.timer.timeout.connect(self.TimerForward)
        self.timer.start()
      else:
        self.timer.stop()

  def TimerForward(self):
    if self.glwidget.kmc:
      if self.glwidget.kmc.step >= self.glwidget.kmc.nevent:
        self.timer.stop()
        self.playb.toggle()
        return
      self.glwidget.kmc.step_forward(1)
      self.glwidget.updateGL()

class ToolButton(QtGui.QToolButton):
  def __init__(self, text, func):
    super(ToolButton, self).__init__()
    self.setText(text)
    self.clicked[bool].connect(func)
    self.setCheckable(True)

if __name__ == '__main__':
  app = QtGui.QApplication(sys.argv)
  window = MainWindow()
  window.show()
  sys.exit(app.exec_())
