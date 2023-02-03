import qetool
import tensorflow as tf
import numpy as np
import pandas as pd
from sklearn.model_selection import train_test_split

def NNModel(Layers, ncluster):
    model = tf.keras.Sequential()
    for i in Layers:
        model.add(tf.keras.layers.Dense(i, activation='relu'))
    model.add(tf.keras.layers.Dense(1))
    model.build(input_shape=(None, ncluster,))
    model.compile(optimizer='Adam', loss='mse', metrics=['mae'])
    return model

def Test(x, y, model, batchsize, epoch, test_size=0.05):
    x_train, x_test, y_train, y_test = train_test_split(x, y, test_size=test_size)
    model.fit(x_train, y_train, batch_size=batchsize, epochs=epoch)
    score = model.evaluate(x_test, y_test, verbose=0)
    print('Test loss:', score[0])
    print('Test accuracy:', score[1])

def Fit(x, y, model, batchsize, epoch, logfile, modelfile, predfile):
    csv_logger = tf.keras.callbacks.CSVLogger(logfile)
    model.fit(x, y, batch_size=batchsize, epochs=epoch, callbacks=[csv_logger])
    model.summary()
    model.save(modelfile)
    y_pred = model.predict(x)
    y_pred = y_pred.reshape(len(x))
    df = pd.DataFrame([y, y_pred], index=['MC', 'Prediction']).T
    df['Diff'] = df['Prediction'] - df['MC']
    df.to_csv(predfile)

if __name__ == "__main__":
    layer = [300, 300,]
    batchsize = 1000
    epoch = 1000
    etbfile = 'Al-Si.etb'
    modelfile = 'Al-Si.h5'
    logfile = 'Al-Si-train.csv'
    predfile = 'Al-Si-pred.csv'
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    kmc.read_table(etbfile)
    kmc.multiple_cluster()
    x, y, replace = qetool.MLData(kmc)
    print('Type replace:', replace)
    model = NNModel(layer, kmc.ncluster)
    #Test(x, y, model, batchsize, epoch)
    Fit(x, y, model, batchsize, epoch, logfile, modelfile, predfile)

