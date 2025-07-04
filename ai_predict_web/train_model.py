# --- This is a NEW, SEPARATE SCRIPT (e.g., train_model.py) ---
import pandas as pd
from sklearn.linear_model import LinearRegression
from sklearn.model_selection import train_test_split
from sklearn.metrics import mean_absolute_error
import joblib # To save/load models

# 1. Load data
df = pd.read_csv('sensor_data_log.csv', parse_dates=['Timestamp'])
df.set_index('Timestamp', inplace=True)

# 2. Feature Engineering: Create lagged features
def create_lagged_features_df(data_df, value_col, n_lags):
    df_lagged = data_df[[value_col]].copy()
    for i in range(1, n_lags + 1):
        df_lagged[f'{value_col}_lag_{i}'] = df_lagged[value_col].shift(i)
    df_lagged['target'] = df_lagged[value_col].shift(-1) # Predict the next value
    df_lagged.dropna(inplace=True)
    
    X = df_lagged.drop(columns=['target', value_col]) # Remove original value and target
    y = df_lagged['target']
    return X, y

N_LAGS = 5 # Number of past data points to use for prediction (e.g., last 5 readings)

# For Temperature
X_temp, y_temp = create_lagged_features_df(df, 'Temperature', N_LAGS)
X_train_temp, X_test_temp, y_train_temp, y_test_temp = train_test_split(X_temp, y_temp, test_size=0.2, shuffle=False)

temp_model_trained = LinearRegression()
temp_model_trained.fit(X_train_temp, y_train_temp)
temp_preds = temp_model_trained.predict(X_test_temp)
print(f"Temperature MAE: {mean_absolute_error(y_test_temp, temp_preds):.2f}")

# For Humidity
X_hum, y_hum = create_lagged_features_df(df, 'Humidity', N_LAGS)
X_train_hum, X_test_hum, y_train_hum, y_test_hum = train_test_split(X_hum, y_hum, test_size=0.2, shuffle=False)

hum_model_trained = LinearRegression()
hum_model_trained.fit(X_train_hum, y_train_hum)
hum_preds = hum_model_trained.predict(X_test_hum)
print(f"Humidity MAE: {mean_absolute_error(y_test_hum, hum_preds):.2f}")

# For Distance
X_dist, y_dist = create_lagged_features_df(df, 'Distance', N_LAGS)
X_train_dist, X_test_dist, y_train_dist, y_test_dist = train_test_split(X_dist, y_dist, test_size=0.2, shuffle=False)

dist_model_trained = LinearRegression()
dist_model_trained.fit(X_train_dist, y_train_dist)
dist_preds = dist_model_trained.predict(X_test_dist)
print(f"Distance MAE: {mean_absolute_error(y_test_dist, dist_preds):.2f}")

# 3. Save the trained models
joblib.dump(temp_model_trained, 'temp_model.pkl')
joblib.dump(hum_model_trained, 'hum_model.pkl')
joblib.dump(dist_model_trained, 'dist_model.pkl')
print("모델 학습 및 저장 완료.")
