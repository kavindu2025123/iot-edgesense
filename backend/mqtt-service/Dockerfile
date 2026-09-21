FROM python:3.12-slim
WORKDIR /app
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt
COPY mqtt_to_database.py .
CMD ["python", "mqtt_to_database.py"]