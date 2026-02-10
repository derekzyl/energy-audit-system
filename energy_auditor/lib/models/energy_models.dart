class EnergyReading {
  final String deviceId;
  final DateTime timestamp;
  final double sensor1Amps;
  final double sensor1Watts;
  final double sensor2Amps;
  final double sensor2Watts;
  final double temperature;
  final double humidity;
  final int lightLux;

  EnergyReading({
    required this.deviceId,
    required this.timestamp,
    required this.sensor1Amps,
    required this.sensor1Watts,
    required this.sensor2Amps,
    required this.sensor2Watts,
    required this.temperature,
    required this.humidity,
    required this.lightLux,
  });

  factory EnergyReading.fromJson(Map<String, dynamic> json) {
    return EnergyReading(
      deviceId: json['device_id'] ?? '',
      timestamp: DateTime.parse(json['timestamp']),
      sensor1Amps: (json['sensor_1_amps'] ?? 0).toDouble(),
      sensor1Watts: (json['sensor_1_watts'] ?? 0).toDouble(),
      sensor2Amps: (json['sensor_2_amps'] ?? 0).toDouble(),
      sensor2Watts: (json['sensor_2_watts'] ?? 0).toDouble(),
      temperature: (json['temperature_c'] ?? 0).toDouble(),
      humidity: (json['humidity_percent'] ?? 0).toDouble(),
      lightLux: json['light_lux'] ?? 0,
    );
  }
}

class EnergyAlert {
  final int id;
  final String deviceId;
  final DateTime timestamp;
  final int sensorNumber;
  final String auditType;
  final String severity;
  final String message;
  final double estimatedWasteWatts;

  EnergyAlert({
    required this.id,
    required this.deviceId,
    required this.timestamp,
    required this.sensorNumber,
    required this.auditType,
    required this.severity,
    required this.message,
    required this.estimatedWasteWatts,
  });

  factory EnergyAlert.fromJson(Map<String, dynamic> json) {
    return EnergyAlert(
      id: json['id'],
      deviceId: json['device_id'],
      timestamp: DateTime.parse(json['timestamp']),
      sensorNumber: json['sensor_number'],
      auditType: json['audit_type'],
      severity: json['severity'],
      message: json['message'],
      estimatedWasteWatts: (json['estimated_waste_watts'] ?? 0).toDouble(),
    );
  }
}
