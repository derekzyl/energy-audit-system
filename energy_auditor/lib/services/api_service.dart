import 'package:dio/dio.dart';
import 'package:flutter/foundation.dart';
import 'package:shared_preferences/shared_preferences.dart';

import '../models/energy_models.dart';

class ApiService {
  final Dio _dio = Dio();
  String? _baseUrl;

  ApiService() {
    _initBaseUrl();
  }

  Future<void> _initBaseUrl() async {
    final prefs = await SharedPreferences.getInstance();
    _baseUrl = prefs.getString('api_url') ?? 'http://192.168.1.100:8000';
    _dio.options.baseUrl = _baseUrl!;
  }

  Future<void> setBaseUrl(String url) async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.setString('api_url', url);
    _baseUrl = url;
    _dio.options.baseUrl = url;
  }

  Future<List<EnergyReading>> getReadings(String deviceId) async {
    await _initBaseUrl();
    try {
      final response = await _dio.get('/energy/readings/$deviceId?limit=20');
      return (response.data as List)
          .map((e) => EnergyReading.fromJson(e))
          .toList();
    } catch (e) {
      debugPrint('Error fetching readings: $e');
      return [];
    }
  }

  Future<List<EnergyAlert>> getAlerts(String deviceId) async {
    await _initBaseUrl();
    try {
      final response = await _dio.get(
        '/energy/audit/$deviceId/alerts?limit=10',
      );
      return (response.data as List)
          .map((e) => EnergyAlert.fromJson(e))
          .toList();
    } catch (e) {
      debugPrint('Error fetching alerts: $e');
      return [];
    }
  }

  Future<void> triggerAudit(String deviceId) async {
    await _initBaseUrl();
    try {
      // Assuming audit triggers on post reading
    } catch (e) {
      debugPrint('Error triggering audit: $e');
    }
  }
}
