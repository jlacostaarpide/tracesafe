import { initializeApp } from 'firebase/app';
import { initializeAuth, getReactNativePersistence } from 'firebase/auth';
import { getFirestore } from 'firebase/firestore';
import { getDatabase } from 'firebase/database';
import AsyncStorage from '@react-native-async-storage/async-storage';

const firebaseConfig = {
  apiKey:            "AIzaSyCcSJW3fbth2GgJv_CXxWH4JESWZN3Wa3s",
  authDomain:        "tracesafe-7f238.firebaseapp.com",
  projectId:         "tracesafe-7f238",
  storageBucket:     "tracesafe-7f238.firebasestorage.app",
  messagingSenderId: "475953945374",
  appId:             "1:475953945374:web:9a018108056988e923070f",
};

const app = initializeApp(firebaseConfig);

export const auth = initializeAuth(app, {
  persistence: getReactNativePersistence(AsyncStorage),
});

export const db   = getFirestore(app);
export const rtdb = getDatabase(app);
