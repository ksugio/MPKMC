import qetool
import tensorflow as tf
import pandas as pd

def NNModel(Layers, ncluster):
    model = tf.keras.Sequential()
    for i in Layers:
        model.add(tf.keras.layers.Dense(i, activation='relu'))
    model.add(tf.keras.layers.Dense(1))
    model.build(input_shape=(None, ncluster,))
    model.compile(optimizer='Adam', loss='mse', metrics=['mae'])
    return model

if __name__ == "__main__":
    Layer = [300, 300, ]
    Batchsize = 1000
    Epoch = 2000
    etbfile = 'Al-Si.etb'
    modelfile = 'Al-Si.h5'
    logfile = 'Al-Si-train.csv'
    predfile = 'Al-Si-pred.csv'
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    kmc.read_table(etbfile)
    print('Multiple Table:', kmc.multiple_cluster())
    x, y, replace = qetool.MLData(kmc)
    print('Type replace:', replace)
    model = NNModel(Layer, kmc.ncluster)
    csv_logger = tf.keras.callbacks.CSVLogger(logfile)
    model.fit(x, y, batch_size=Batchsize, epochs=Epoch, callbacks=[csv_logger])
    model.summary()
    model.save(modelfile)
    y_pred = model.predict(x)
    y_pred = y_pred.reshape(len(x))
    df = pd.DataFrame([y, y_pred], index=['MC', 'Prediction']).T
    df['Diff'] = df['Prediction'] - df['MC']
    df.to_csv(predfile)
