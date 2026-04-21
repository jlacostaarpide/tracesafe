import React, { useState, useEffect, useRef } from 'react';
import {
  View, Text, TouchableOpacity, StyleSheet, Alert,
  Modal, SafeAreaView, Animated, Easing, ActivityIndicator,
} from 'react-native';
import { signOut } from 'firebase/auth';
import { ref, onValue, update } from 'firebase/database';
import * as Notifications from 'expo-notifications';
import { auth, rtdb } from '../firebaseConfig';

export default function HomeScreen() {
  const [pendingRequest, setPendingRequest] = useState(null);
  const [modalVisible, setModalVisible]     = useState(false);
  const [processing, setProcessing]         = useState(false);
  const pulseAnim = useRef(new Animated.Value(1)).current;
  const user = auth.currentUser;

  // Animación de pulso
  useEffect(() => {
    const loop = Animated.loop(
      Animated.sequence([
        Animated.timing(pulseAnim, { toValue: 1.18, duration: 850, easing: Easing.inOut(Easing.ease), useNativeDriver: true }),
        Animated.timing(pulseAnim, { toValue: 1,    duration: 850, easing: Easing.inOut(Easing.ease), useNativeDriver: true }),
      ])
    );
    loop.start();
    return () => loop.stop();
  }, []);

  // Permisos de notificación
  useEffect(() => {
    Notifications.requestPermissionsAsync();
  }, []);

  // Escuchar solicitudes pendientes del ESP32 en Realtime Database
  useEffect(() => {
    const requestRef = ref(rtdb, 'access_requests');
    const unsub = onValue(requestRef, (snapshot) => {
      const data = snapshot.val();
      // Solo actúa si la solicitud es para este usuario concreto
      if (data && data.status === 'pending' && data.userId === user.uid) {
        setPendingRequest(data);
        setModalVisible(true);
        Notifications.scheduleNotificationAsync({
          content: {
            title: '🚨 Solicitud de acceso',
            body: `${data.userName || 'Alguien'} quiere entrar. Abre la app para autorizar.`,
          },
          trigger: null,
        });
      }
    });
    return () => unsub();
  }, []);

  const handleResponse = async (approved) => {
    if (!pendingRequest || processing) return;
    setProcessing(true);
    try {
      await update(ref(rtdb, 'access_requests'), {
        status:      approved ? 'approved' : 'denied',
        respondedAt: Date.now(),
        respondedBy: user.uid,
      });
      setModalVisible(false);
      setPendingRequest(null);
      Alert.alert(
        approved ? '✅ Acceso autorizado' : '❌ Acceso denegado',
        approved ? 'La puerta se abrirá.' : 'LEDs rojos en el ESP32.'
      );
    } catch {
      Alert.alert('Error', 'No se pudo enviar la respuesta.');
    } finally {
      setProcessing(false);
    }
  };

  const handleLogout = () =>
    Alert.alert('Cerrar sesión', '¿Seguro?', [
      { text: 'Cancelar', style: 'cancel' },
      { text: 'Salir', style: 'destructive', onPress: () => signOut(auth) },
    ]);

  return (
    <SafeAreaView style={styles.container}>

      {/* Cabecera */}
      <View style={styles.header}>
        <View>
          <Text style={styles.headerTitle}>TraceSafe</Text>
          <Text style={styles.headerSub}>{user?.email}</Text>
        </View>
        <TouchableOpacity style={styles.logoutBtn} onPress={handleLogout}>
          <Text style={styles.logoutText}>Salir</Text>
        </TouchableOpacity>
      </View>

      {/* Badge de estado */}
      <View style={styles.statusCard}>
        <View style={styles.dot} />
        <View>
          <Text style={styles.statusTitle}>Sistema activo</Text>
          <Text style={styles.statusSub}>Escuchando solicitudes de acceso…</Text>
        </View>
      </View>

      {/* Zona de espera */}
      <View style={styles.center}>
        <Animated.Text style={[styles.radarIcon, { transform: [{ scale: pulseAnim }] }]}>
          📡
        </Animated.Text>
        <Text style={styles.waitTitle}>En espera</Text>
        <Text style={styles.waitText}>
          Cuando el ESP32 detecte tu brazalete recibirás aquí una solicitud para autorizar o denegar el acceso.
        </Text>
      </View>

      {/* ── MODAL DE SOLICITUD ── */}
      <Modal visible={modalVisible} transparent animationType="slide" onRequestClose={() => {}}>
        <View style={styles.overlay}>
          <View style={styles.modalCard}>

            <Text style={styles.modalEmoji}>🚨</Text>
            <Text style={styles.modalTitle}>Solicitud de acceso</Text>

            {pendingRequest && (
              <View style={styles.infoBox}>
                <InfoRow label="Usuario"   value={pendingRequest.userName} />
                <InfoRow label="Brazalete" value={pendingRequest.braceletId} />
                <InfoRow label="Hora"      value={new Date().toLocaleTimeString('es-ES')} />
              </View>
            )}

            <Text style={styles.question}>¿Autorizas la entrada?</Text>

            {processing
              ? <ActivityIndicator size="large" color="#00d4ff" style={{ marginTop: 20 }} />
              : (
                <View style={styles.btnRow}>
                  <TouchableOpacity style={[styles.actionBtn, styles.denyBtn]}    onPress={() => handleResponse(false)}>
                    <Text style={styles.actionTxt}>✗  Denegar</Text>
                  </TouchableOpacity>
                  <TouchableOpacity style={[styles.actionBtn, styles.approveBtn]} onPress={() => handleResponse(true)}>
                    <Text style={styles.actionTxt}>✓  Autorizar</Text>
                  </TouchableOpacity>
                </View>
              )
            }

          </View>
        </View>
      </Modal>

    </SafeAreaView>
  );
}

function InfoRow({ label, value }) {
  return (
    <View style={styles.infoRow}>
      <Text style={styles.infoLabel}>{label}</Text>
      <Text style={styles.infoValue}>{value}</Text>
    </View>
  );
}

const styles = StyleSheet.create({
  container:   { flex: 1, backgroundColor: '#1a1a2e' },

  header: {
    flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center',
    backgroundColor: '#16213e', paddingHorizontal: 20, paddingVertical: 14,
    paddingTop: 18, borderBottomWidth: 1, borderBottomColor: '#0f3460',
  },
  headerTitle: { fontSize: 20, fontWeight: 'bold', color: '#00d4ff' },
  headerSub:   { fontSize: 12, color: '#444', marginTop: 2 },
  logoutBtn:   { backgroundColor: '#c0392b', paddingHorizontal: 16, paddingVertical: 8, borderRadius: 10 },
  logoutText:  { color: '#fff', fontWeight: 'bold' },

  statusCard: {
    flexDirection: 'row', alignItems: 'center',
    backgroundColor: '#16213e', margin: 20, padding: 18,
    borderRadius: 16, borderLeftWidth: 4, borderLeftColor: '#00ff88',
  },
  dot:         { width: 12, height: 12, borderRadius: 6, backgroundColor: '#00ff88', marginRight: 14 },
  statusTitle: { color: '#00ff88', fontSize: 15, fontWeight: 'bold' },
  statusSub:   { color: '#444', fontSize: 12, marginTop: 2 },

  center:     { flex: 1, justifyContent: 'center', alignItems: 'center', paddingHorizontal: 40 },
  radarIcon:  { fontSize: 72, marginBottom: 24 },
  waitTitle:  { fontSize: 22, fontWeight: 'bold', color: '#fff', marginBottom: 12 },
  waitText:   { fontSize: 15, color: '#555', textAlign: 'center', lineHeight: 24 },

  overlay: { flex: 1, backgroundColor: 'rgba(0,0,0,0.85)', justifyContent: 'center', alignItems: 'center', padding: 24 },
  modalCard: {
    backgroundColor: '#16213e', borderRadius: 28, padding: 30,
    width: '100%', maxWidth: 420, alignItems: 'center',
    borderWidth: 2, borderColor: '#00d4ff',
    shadowColor: '#00d4ff', shadowOffset: { width: 0, height: 0 }, shadowOpacity: 0.5, shadowRadius: 20, elevation: 20,
  },
  modalEmoji: { fontSize: 54, marginBottom: 10 },
  modalTitle: { fontSize: 24, fontWeight: 'bold', color: '#00d4ff', marginBottom: 22 },

  infoBox:   { width: '100%', backgroundColor: '#0f3460', borderRadius: 14, padding: 14, marginBottom: 22 },
  infoRow:   { flexDirection: 'row', justifyContent: 'space-between', paddingVertical: 7, borderBottomWidth: 1, borderBottomColor: '#1e4d8c' },
  infoLabel: { color: '#666', fontSize: 14 },
  infoValue: { color: '#fff', fontSize: 14, fontWeight: '600' },

  question: { fontSize: 17, color: '#ccc', marginBottom: 26, textAlign: 'center' },

  btnRow:     { flexDirection: 'row', gap: 14, width: '100%' },
  actionBtn:  { flex: 1, paddingVertical: 18, borderRadius: 16, alignItems: 'center' },
  approveBtn: { backgroundColor: '#00c875' },
  denyBtn:    { backgroundColor: '#e74c3c' },
  actionTxt:  { color: '#fff', fontSize: 17, fontWeight: 'bold' },
});
