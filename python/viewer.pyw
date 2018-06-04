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
    self.kmc = None
    self.draw = MPGLKMC.draw()
    self.scene = MPGLKMC.scene()
    self.scene.light_add(1.0, 1.0, 1.0, 0.0)
    self.scene.proj = 0    
    self.model = MPGLKMC.model()
    self.cmp = MPGLKMC.colormap()
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
      GL.glPushMatrix()
      self.model.transform()
      self.draw.draw_grid(self.kmc, self.cmp)
      self.draw.draw_frame(self.kmc)     
      GL.glTranslatef(-0.5, -0.5, -0.5)
      self.draw.draw_axis(self.kmc.size)
      GL.glPopMatrix()
      GL.glPushMatrix()
      GL.glRotated(90.0, 0.0, 0.0, 1.0)
      GL.glRotated(90.0, 1.0, 0.0, 0.0)
      GL.glTranslate((2.0 - self.__width) / self.__height, -self.cmp.size[1]/2.0, self.scene.znear - 1.0e-6)
      self.cmp.draw()
      GL.glPopMatrix()
      stx = (2.0 * 10 - self.__width) / self.__height
      sty = 2.0*(self.__height - 20) / self.__height - 1.0
      self.drawString(stx, sty, str(self.kmc.step) + ' step')

  def resizeGL(self, width, height):
    self.__width = width
    self.__height = height
    self.scene.resize(width, height) 

  def mousePressEvent(self, event):
    self.lastPos = QtCore.QPoint(event.pos())    

  def mouseReleaseEvent(self, event):
    self.model.inverse()

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
                self.model.rot_x(-ax)
            else:
                az = math.pi * dx / self.__height
                self.model.rot_z(-az)
                ay = math.pi *dy / self.__height
                self.model.rot_y(-ay)
        elif self.mouseMode == 1:
            if mod == QtCore.Qt.ControlModifier:
                mx = 2.0 * dy / self.__height
                self.model.trans_x(-mx)
            else:
                my = 2.0 * dx / self.__height
                self.model.trans_y(my)
                mz = 2.0 * dy / self.__height
                self.model.trans_z(-mz)
        elif self.mouseMode == 2:
            s = 1.0 - float(dy) / self.__height
            self.model.zoom(s)
        self.updateGL()
        self.lastPos = QtCore.QPoint(event.pos())

  def fitModel(self):
    region = self.draw.region(self.kmc)
    self.model.fit_center(region)
    aspect = float(self.__width)/float(self.__height)
    self.model.fit_scale(region, aspect)

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

"""
NewKMCDialog
""" 
class NewKMCDialog(QtGui.QDialog):
  def __init__(self, parent):
    QtGui.QDialog.__init__(self, parent)
    self.setWindowTitle("New KMC")
    vbox = QtGui.QVBoxLayout(self)
    hbox0 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox0)
    label0 = QtGui.QLabel()
    label0.setText('Lattice Type')
    hbox0.addWidget(label0)
    self.combo0 = QtGui.QComboBox()
    self.combo0.addItem('FCC')
    hbox0.addWidget(self.combo0)
    hbox1 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox1)
    label1 = QtGui.QLabel()
    label1.setText('Nx, Ny, Nz')
    hbox1.addWidget(label1)
    self.spinnx = self.SpinBox(2000, 1)
    hbox1.addWidget(self.spinnx)
    self.spinny = self.SpinBox(2000, 1)
    hbox1.addWidget(self.spinny)
    self.spinnz = self.SpinBox(2000, 1)
    hbox1.addWidget(self.spinnz)
    hbox2 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox2)
    label2 = QtGui.QLabel()
    label2.setText('Solvent')
    hbox2.addWidget(label2)
    self.spinmatrix = self.SpinBox(100, 1)
    hbox2.addWidget(self.spinmatrix)
    hbox3 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox3)
    label3 = QtGui.QLabel()
    label3.setText('NSolute Max')
    hbox3.addWidget(label3)
    self.nsolute_max = self.SpinBox(100000, 1000)
    hbox3.addWidget(self.nsolute_max)    
    hbox4 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox4)
    label4 = QtGui.QLabel()
    label4.setText('NTable Step')
    hbox4.addWidget(label4)
    self.ntable_step = self.SpinBox(100000, 1000)
    hbox4.addWidget(self.ntable_step)
    hbox5 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox5)
    label5 = QtGui.QLabel()
    label5.setText('NEvent Step')
    hbox5.addWidget(label5)
    self.nevent_step = self.SpinBox(1000000, 10000)
    hbox5.addWidget(self.nevent_step)
    self.button = QtGui.QDialogButtonBox()
    vbox.addWidget(self.button)
    self.button.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
    self.button.accepted.connect(self.accept)
    self.button.rejected.connect(self.reject)

  def SpinBox(self, max, val):
    spin = QtGui.QSpinBox()
    spin.setMinimum(1)
    spin.setMaximum(max)
    spin.setValue(val)
    spin.setMinimumWidth(50)
    return spin

  @staticmethod
  def newKMC(self):
    dlg = NewKMCDialog(self)
    ok = dlg.exec_()
    if ok:
      lat_type = dlg.combo0.currentIndex()
      nx = dlg.spinnx.value()
      ny = dlg.spinny.value()
      nz = dlg.spinnz.value()
      solvent = dlg.spinmatrix.value()
      nsolute_max = dlg.nsolute_max.value()
      ntable_step = dlg.ntable_step.value()
      nevent_step = dlg.nevent_step.value()
      kmc = MPKMC.new(lat_type, nx, ny, nz, solvent, nsolute_max, ntable_step, nevent_step)
      return kmc
    else:
      return None

"""
AddSoluteDialog
""" 
class AddSoluteDialog(QtGui.QDialog):
  def __init__(self, parent, kmc):
    QtGui.QDialog.__init__(self, parent)
    self.kmc = kmc
    self.setWindowTitle("Add Solute")
    vbox = QtGui.QVBoxLayout(self)
    hbox1 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox1)
    label1 = QtGui.QLabel()
    label1.setText('X, Y, Z')
    hbox1.addWidget(label1)
    sx, sy, sz = kmc.size
    self.spinx = self.SpinBox(sx, 0)
    hbox1.addWidget(self.spinx)
    self.spiny = self.SpinBox(sy, 0)
    hbox1.addWidget(self.spiny)
    self.spinz = self.SpinBox(sz, 0)
    hbox1.addWidget(self.spinz)
    hbox2 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox2)
    label2 = QtGui.QLabel()
    label2.setText('Type')
    hbox2.addWidget(label2)
    self.type = self.SpinBox(200, 1)
    hbox2.addWidget(self.type)
    self.jump = QtGui.QCheckBox("Jump")
    self.jump.setChecked(True)
    vbox.addWidget(self.jump)
    self.buttonb = QtGui.QDialogButtonBox()
    vbox.addWidget(self.buttonb)
    self.buttonb.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
    self.buttonb.accepted.connect(self.Accept)
    self.buttonb.rejected.connect(self.reject)

  def SpinBox(self, max, val):
    spin = QtGui.QSpinBox()
    spin.setMinimum(0)
    spin.setMaximum(max)
    spin.setValue(val)
    spin.setMinimumWidth(50)
    return spin

  def Accept(self):
    x = self.spinx.value()
    y = self.spiny.value()
    z = self.spinz.value()
    i = self.kmc.grid2index(x, y, z)
    tp = self.type.value()
    if self.jump.isChecked():
      self.kmc.add_solute(i, tp, 1)
    else:
      self.kmc.add_solute(i, tp, 0)
    self.accept()

"""
AddSoluteRandomDialog
""" 
class AddSoluteRandomDialog(QtGui.QDialog):
  def __init__(self, parent, kmc):
    QtGui.QDialog.__init__(self, parent)
    self.kmc = kmc
    self.setWindowTitle("Add Solute Random")
    vbox = QtGui.QVBoxLayout(self)
    hbox1 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox1)
    label1 = QtGui.QLabel()
    label1.setText('Number')
    hbox1.addWidget(label1)
    self.number = self.SpinBox(kmc.nsolute_max, 1)
    hbox1.addWidget(self.number)
    hbox2 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox2)
    label2 = QtGui.QLabel()
    label2.setText('Type')
    hbox2.addWidget(label2)
    self.type = self.SpinBox(200, 1)
    hbox2.addWidget(self.type)
    self.jump = QtGui.QCheckBox("Jump")
    self.jump.setChecked(True)
    vbox.addWidget(self.jump)
    self.buttonb = QtGui.QDialogButtonBox()
    vbox.addWidget(self.buttonb)
    self.buttonb.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
    self.buttonb.accepted.connect(self.Accept)
    self.buttonb.rejected.connect(self.reject)

  def SpinBox(self, max, val):
    spin = QtGui.QSpinBox()
    spin.setMinimum(0)
    spin.setMaximum(max)
    spin.setValue(val)
    spin.setMinimumWidth(50)
    return spin

  def Accept(self):
    num = self.number.value()
    tp = self.type.value()
    if self.jump.isChecked():
      self.kmc.add_solute_random(num, tp, 1)
    else:
      self.kmc.add_solute_random(num, tp, 0)    
    self.accept()

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
    self.table = QtGui.QTableWidget(kmc.ntypes, 2)
    self.table.setHorizontalHeaderLabels(["Type", "Display"])
    for i in range(kmc.ntypes):
      self.table.setItem(i, 0, QtGui.QTableWidgetItem(str(kmc.types(i))))
      item = QtGui.QTableWidgetItem()
      if draw.get_disp(i) == 0:
        item.setCheckState(QtCore.Qt.Unchecked)
      else:
        item.setCheckState(QtCore.Qt.Checked)
      self.table.setItem(i, 1, item)
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
    file_menu.addAction('New', self.fileNew)   
    file_menu.addAction('Open', self.fileOpen)
    file_menu.addAction('Save', self.fileSave)
    file_menu.addAction('Save Image', self.fileSaveImage)
    file_menu.addAction('Quit', QtCore.QCoreApplication.instance().quit)   
    menubar.addMenu(file_menu)
    view_menu = QtGui.QMenu('View', self)
    view_menu.addAction('Set Display', self.setDispDialog)
    view_menu.addAction('Set Shift', self.setShiftDialog)   
    menubar.addMenu(view_menu)
    grid_menu = QtGui.QMenu('Grid', self)
    grid_menu.addAction('Add Solute', self.addSoluteDialog)    
    grid_menu.addAction('Add Solute Random', self.addSoluteRandomDialog)
    menubar.addMenu(grid_menu)

  def fileNew(self):
    self.glwidget.kmc = NewKMCDialog.newKMC(self)
    if self.glwidget.kmc:
      self.glwidget.model.init()
      self.glwidget.fitModel()
      self.glwidget.cmp.range = (0.0, 0.0)
      self.glwidget.updateGL()

  def fileOpen(self):
    fname = QtGui.QFileDialog.getOpenFileName(self, 'Open file')
    if fname:
      self.glwidget.kmc = MPKMC.read(str(fname))
      self.glwidget.model.init()
      self.glwidget.fitModel()
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
    toolbar.addSeparator()  
    toolbar.addAction('Backward', self.StepBackward)
    toolbar.addAction('Forward', self.StepForward)
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
        self.glwidget.draw.kind = 0
    elif txt == "Energy":
        self.glwidget.draw.kind = 1
    self.glwidget.updateGL()

  def initModel(self):
    self.glwidget.model.init()
    self.glwidget.updateGL()

  def fitModel(self):
    self.glwidget.fitModel()
    self.glwidget.updateGL()

  def StepBackward(self):
    if self.glwidget.kmc:
      self.glwidget.kmc.step_backward(1)
      self.glwidget.updateGL() 
    
  def StepForward(self):
    if self.glwidget.kmc:
      self.glwidget.kmc.step_forward(1)
      self.glwidget.updateGL()    

  def StepGo(self):
    if self.glwidget.kmc:
      dlg = StepGoDialog(self, self.glwidget.kmc)
      dlg.exec_()    

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
