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
    self.dispMode = 0
    self.kmc = None
    self.draw = MPGLKMC.draw()
    self.scene = MPGLKMC.scene()
    self.scene.light_add(1.0, 1.0, 1.0, 0.0)
    self.scene.proj = 1    
    self.model = [None, None]
    self.cmp = MPGLKMC.colormap()
    self.tab = None
    self.tabid = 0

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
        self.draw.atoms(self.kmc, self.cmp)
        GL.glPopMatrix()
        self.drawColormap()
        s = str(self.kmc.step) + ' step'
        self.drawString(10, 20, s)
      elif self.dispMode == 1:
        GL.glPushMatrix()
        self.model[1].transform()
        item = self.kmc.table_item(self.tabid)
        self.draw.cluster(self.kmc, self.cmp, item[0])
        GL.glPopMatrix()
        self.drawColormap()
        s = 'ID %d,   Energy %f,   Count %d' % (self.tabid, item[1], item[2])
        self.drawString(10, 20, s)

  def drawColormap(self):
    GL.glPushMatrix()
    GL.glRotated(90.0, 0.0, 0.0, 1.0)
    GL.glRotated(90.0, 1.0, 0.0, 0.0)
    GL.glTranslate((2.0 - self.scene.width) / self.scene.height, -self.cmp.size[1]/2.0, self.scene.znear - 1.0e-6)
    self.cmp.draw()
    GL.glPopMatrix()

  def drawString(self, x, y, s):
    GL.glPushAttrib(GL.GL_LIGHTING_BIT)
    GL.glDisable(GL.GL_LIGHTING)
    GL.glColor3fv(self.cmp.font_color)
    self.scene.front_text(x, y, s, self.cmp.font_type)
    GL.glPopAttrib()
    
  def resizeGL(self, width, height):
    self.scene.resize(width, height) 

  def mousePressEvent(self, event):
    if self.model[self.dispMode]:
      self.model[self.dispMode].button(event.x(), event.y(), 1)

  def mouseReleaseEvent(self, event):
    if self.model[self.dispMode]:
      self.model[self.dispMode].button(event.x(), event.y(), 0)

  def mouseMoveEvent(self, event):
    if self.model[self.dispMode]:
      if event.buttons() & QtCore.Qt.LeftButton:
        if QtGui.QApplication.keyboardModifiers() == QtCore.Qt.ControlModifier:
          ctrl = 1 
        else:
          ctrl = 0
        if self.model[self.dispMode].motion(self.scene, event.x(), event.y(), ctrl):
          self.updateGL()

  def screenShot(self):
    screenshot = GL.glReadPixels(0, 0, self.width(), self.height(), GL.GL_RGBA, GL.GL_UNSIGNED_BYTE)
    img = Image.frombuffer("RGBA", (self.width(), self.height()), screenshot, "raw", "RGBA", 0, 0)
    return img

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
SearchTableDialog
"""
class SearchTableDialog(QtGui.QDialog):
  def __init__(self, parent, kmc, tab):
    QtGui.QDialog.__init__(self, parent)
    self.kmc = kmc
    self.tab = tab
    self.setWindowTitle("Search Table")
    vbox = QtGui.QVBoxLayout(self)
    hbox1 = QtGui.QHBoxLayout()
    vbox.addLayout(hbox1)
    self.line1 = QtGui.QLineEdit()
    hbox1.addWidget(self.line1)
    self.button1 = QtGui.QPushButton('Search')
    self.button1.clicked[bool].connect(self.searchTable)
    hbox1.addWidget(self.button1)
    self.list1 = QtGui.QListWidget()
    vbox.addWidget(self.list1)

  def searchTable(self):
    self.tab = self.kmc.search_table(str(self.line1.text()))
    nlist = self.tab[0]
    item = self.tab[1]
    self.list1.clear()
    for i in range(nlist):
      it = '%d %f %d' % (i, item[i][1], item[i][2])
      self.list1.addItem(it)
    
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
    view_menu.addAction('Search Table', self.searchTableDialog)
    menubar.addMenu(view_menu)

  def fileOpen(self):
    fname = QtGui.QFileDialog.getOpenFileName(self, 'Open file')
    if fname:
      self.glwidget.kmc = MPKMC.read(str(fname))
      self.glwidget.kmc.total_energy(None)
      region = self.glwidget.draw.atoms_region(self.glwidget.kmc)
      self.glwidget.model[0] = MPGLKMC.model((1,0,0,0,0,1), region)
      region = self.glwidget.draw.cluster_region(self.glwidget.kmc)
      self.glwidget.model[1] = MPGLKMC.model((1,0,0,0,0,1), region)
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

  def searchTableDialog(self):
    if self.glwidget.kmc:
      dlg = SearchTableDialog(self, self.glwidget.kmc, self.glwidget.tab)
      dlg.exec_()
      
  def MainToolBar(self):
    toolbar = QtGui.QToolBar(self)
    toolbar.addAction('Reset', self.resetModel)
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
    toolbar.addAction('<<', self.stepFirst)    
    toolbar.addAction('<', self.stepBackward)
    self.playb = ToolButton('Play', self.stepPlay)    
    toolbar.addWidget(self.playb)
    toolbar.addAction('>', self.stepForward)
    toolbar.addAction('>>', self.stepLast) 
    toolbar.addAction('Go', self.stepGo)
    toolbar.addSeparator()
    toolbar.addAction('TB-', self.tableDown)    
    toolbar.addAction('TB+', self.tableUp)
    self.addToolBar(toolbar)

  def setMouseMode(self, pressed):
    txt = self.sender().text()
    if txt == "Rot":
        self.glwidget.model[self.glwidget.dispMode].button_mode = 0
    elif txt == "Trans":
        self.glwidget.model[self.glwidget.dispMode].button_mode = 1
    elif txt == "Zoom":
        self.glwidget.model[self.glwidget.dispMode].button_mode = 2

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

  def resetModel(self):
    self.glwidget.model[self.glwidget.dispMode].reset()
    self.glwidget.updateGL()

  def fitModel(self):
    self.glwidget.model[self.glwidget.dispMode].fit()
    self.glwidget.updateGL()

  def stepFirst(self):
    if self.glwidget.kmc:
      self.glwidget.kmc.step_go(0)
      self.glwidget.updateGL()
      
  def stepBackward(self):
    if self.glwidget.kmc:
      self.glwidget.kmc.step_backward(1)
      self.glwidget.updateGL() 
    
  def stepForward(self):
    if self.glwidget.kmc:
      self.glwidget.kmc.step_forward(1)
      self.glwidget.updateGL()

  def stepLast(self):
    if self.glwidget.kmc:
      self.glwidget.kmc.step_go(self.kmc.nevent)
      self.glwidget.updateGL()

  def stepGo(self):
    if self.glwidget.kmc:
      dlg = StepGoDialog(self, self.glwidget.kmc)
      dlg.exec_()    

  def stepPlay(self):
    if self.glwidget.dispMode == 0:
      if self.playb.isChecked():
        self.timer = QtCore.QTimer()
        self.timer.setInterval(10)
        self.timer.timeout.connect(self.timerForward)
        self.timer.start()
      else:
        self.timer.stop()

  def timerForward(self):
    if self.glwidget.kmc:
      if self.glwidget.kmc.step >= self.glwidget.kmc.nevent:
        self.timer.stop()
        self.playb.toggle()
        return
      self.glwidget.kmc.step_forward(1)
      self.glwidget.updateGL()
      
  def tableDown(self):
    if self.glwidget.kmc and self.glwidget.tabid > 0:
      self.glwidget.tabid = self.glwidget.tabid - 1
      self.glwidget.updateGL()

  def tableUp(self):
    if self.glwidget.kmc and self.glwidget.tabid < self.glwidget.kmc.ntable - 1 :
      self.glwidget.tabid = self.glwidget.tabid + 1
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
