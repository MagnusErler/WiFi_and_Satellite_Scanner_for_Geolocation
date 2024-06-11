import { useState, useEffect, useRef } from "react";
import axios from "axios";

import "./styles.css";
import "leaflet/dist/leaflet.css";

import MapComponent from "./components/MapComponent";
import SettingsMenu from "./components/SettingsMenu";
import AddDeviceMenu from "./components/AddDeviceMenu";
import MapTileMenu from "./components/MapTileMenu";

import ListIcon from "@mui/icons-material/List";
import AddIcon from "@mui/icons-material/Add";
import LayersIcon from "@mui/icons-material/Layers";

export default function App() {
  const [markers, setMarkers] = useState([]);
  const [trackerInformation, setTrackerInformation] = useState([]);

  useEffect(() => {
    fetchTrackerInformation();
  }, []);

  const center = [56.234538, 10.231792]; // Denmark coordinates
  const zoomLevel = 7;
  const drawMarkersAtZoomLevel = 16;

  const [showCurrentLocation, setShowCurrentLocation] = useState([]);
  const [showMovement, setShowMovement] = useState([]);
  const [selectedMapTile, setSelectedMapTile] = useState("CartoDB_Voyager");

  const [isSettingsMenuOpen, setSettingsMenuOpen] = useState(false);
  const [isAddDeviceMenuOpen, setAddDeviceMenuOpen] = useState(false);
  const [isMapTileMenuOpen, setMapTileMenuOpen] = useState(false);
  const settingsMenuRef = useRef(null);
  const addDeviceMenuRef = useRef(null);
  const mapTileMenuRef = useRef(null);

  const [dataLoaded, setDataLoaded] = useState(false);

  useEffect(() => {
    console.log("Version 11/06/2023 - 20:34");
  }, []);

  // Function to handle setting showCurrentLocation once all data is loaded
  useEffect(() => {
    // Check if both markers and tracker information have been fetched
    if (markers.length > 0 && trackerInformation.length > 0 && !dataLoaded) {
      // Extract all IDs from trackerInformation
      const allIds = trackerInformation.map((info) =>
        info.deviceId.toUpperCase()
      );
      // Set showCurrentLocation to contain all IDs
      setShowCurrentLocation(allIds);
      // Update dataLoaded state to prevent re-execution of this useEffect
      setDataLoaded(true);
    }
  }, [markers, trackerInformation, dataLoaded]);

  const handleOpenSettingsMenu = () => {
    setSettingsMenuOpen(true);
  };

  const handleCloseSettingsMenu = () => {
    setSettingsMenuOpen(false);
  };

  const handleOpenAddDeviceMenu = () => {
    setAddDeviceMenuOpen(true);
  };

  const handleCloseAddDeviceMenu = () => {
    setAddDeviceMenuOpen(false);
  };

  const handleOpenMapTileMenu = () => {
    setMapTileMenuOpen(true);
  };

  const handleCloseMapTileMenu = () => {
    setMapTileMenuOpen(false);
  };

  const handleTileChange = (tile) => {
    setSelectedMapTile(tile);
    console.log(`Map tile changed to: ${tile}`);
  };

  const handleShowLocationSwitch = (ids) => {
    // Convert all IDs to lowercase before setting the state
    const uppercaseIds = ids.map((id) => id.toUpperCase());
    setShowCurrentLocation(uppercaseIds);
  };

  const handleShowMovementSwitch = (ids) => {
    // Convert all IDs to lowercase before setting the state
    const uppercaseIds = ids.map((id) => id.toUpperCase());
    setShowMovement(uppercaseIds);
  };

  // Function to handle successful addition of device and refetch tracker information
  const handleAddDeviceSuccess = async () => {
    try {
      // Fetch tracker information again
      await fetchTrackerInformation();
    } catch (error) {
      console.error("Error fetching tracker information:", error);
      // Handle error fetching tracker information
    }
  };

  const getDeviceInfo = async () => {
    try {
      const response = await axios.get("/api/getDeviceInfoFromJoinserver");
      console.log(response.data);
      // Process device information as needed
    } catch (error) {
      console.error("Failed to fetch device information:", error);
      alert("Failed to fetch device information. Check console for details.");
    }
  };

  // Function to filter the markers so that only the newest marker for each ID is included
  const filterNewestMarkers = (markers) => {
    const newestMarkers = {};
    markers.forEach((marker) => {
      if (
        !newestMarkers[marker.deviceId] ||
        marker.timeStamp > newestMarkers[marker.deviceId].timeStamp
      ) {
        newestMarkers[marker.deviceId] = marker;
      }
    });
    return Object.values(newestMarkers);
  };

  // Update filtered markers when toggling switches
  const filteredCurrentLocationMarkers = filterNewestMarkers(
    markers.filter((marker) => showCurrentLocation.includes(marker.deviceId))
  );
  const filteredAllLocationMarkers = markers.filter((marker) =>
    showMovement.includes(marker.deviceId)
  );

  // Combine all markers
  const allMarkers = [
    ...filteredCurrentLocationMarkers,
    ...filteredAllLocationMarkers,
  ];

  // Function to filter out duplicates from all markers
  const filterUniqueMarkers = (markers) => {
    const uniqueMarkers = {};
    markers.forEach((marker) => {
      if (!uniqueMarkers[marker.pingId]) {
        uniqueMarkers[marker.pingId] = marker;
      }
    });
    return Object.values(uniqueMarkers);
  };

  // Filter out duplicates
  const filteredAllMarkers = filterUniqueMarkers(allMarkers);

  const handleTrackerInformationUpdate = (updatedTrackerInformation) => {
    setTrackerInformation(updatedTrackerInformation);
  };

  const fetchMarkers = async () => {
    console.log("Fetching markers...");
    try {
      // Initialize an array to store the markers
      const updatedMarkers = [];

      // Create an array of promises for fetching markers for each device
      const fetchMarkerPromises = trackerInformation.map(async (device) => {
        const deviceId = device.deviceId.toUpperCase();
        try {
          const response = await axios.get(
            `/api/geolocationSolves/${deviceId}`
          );

          // Map pings to the desired format
          const mappedMarkers = response.data.map((ping) => ({
            pingId: ping.id,
            deviceId: ping.deviceID,
            geocode: ping.geocode,
            timeStamp: new Date(ping.updatedAt).toLocaleString(), // Convert to human-readable format
            accuracy: ping.accuracy,
          }));

          // Add the mapped markers to the updatedMarkers array
          updatedMarkers.push(...mappedMarkers);
          console.log(`Markers for device ${deviceId}:`, response.data);
        } catch (error) {
          // Handle 404 error specifically
          if (error.response && error.response.status === 404) {
            console.log(`No entries found for device ${deviceId}`);
          } else {
            // Handle other errors
            console.error(
              `Failed to fetch markers for device ${deviceId}:`,
              error
            );
          }
        }
      });

      // Wait for all fetchMarkerPromises to complete
      await Promise.all(fetchMarkerPromises);

      // Update the markers state with the updatedMarkers array
      console.log("Markers fetched successfully:", updatedMarkers);
      setMarkers(updatedMarkers);
    } catch (error) {
      console.error("Failed to fetch markers:", error);
      alert("Failed to fetch markers. Check console for details.");
    }
  };

  const fetchTrackerInformation = async () => {
    console.log("Fetching tracker information...");
    try {
      const token = process.env.REACT_APP_TTN_API_KEY;
      const appID = process.env.REACT_APP_TTN_APP_ID;
      if (!token) {
        throw new Error("API token ID is not available.");
      }
      if (!appID) {
        throw new Error("Application ID is not available.");
      }
      const response = await fetch(
        `https://eu1.cloud.thethings.network/api/v3/applications/${appID}/devices?field_mask=name,description`,
        {
          method: "GET",
          headers: {
            Authorization: `Bearer ${token}`,
          },
        }
      );

      if (response.ok) {
        const data = await response.json();
        console.log(data);

        if (data && data.end_devices) {
          const updatedTrackerInformation = await Promise.all(
            data.end_devices.map(async (newDevice) => {
              const deviceIdWithoutPrefix = newDevice.ids.device_id.replace(
                "eui-",
                ""
              );
              const existingDevice = trackerInformation.find(
                (existingDevice) =>
                  existingDevice.deviceId === deviceIdWithoutPrefix
              );

              let batteryStatus = "-";
              let temperature = "-";
              let updateInterval = "-";

              try {
                const deviceResponse = await axios.get(
                  `/api/trackerinformation/${deviceIdWithoutPrefix.toUpperCase()}`
                );
                console.log(`Tracker information for device ${deviceIdWithoutPrefix}:`, deviceResponse.data);
                batteryStatus = deviceResponse.data.batteryStatus ? `${parseFloat(deviceResponse.data.batteryStatus).toFixed(2)}\xa0V` : "-";
                temperature = deviceResponse.data.temp ? `${deviceResponse.data.temp}\xa0°C` : "-";
                updateInterval = deviceResponse.data.updateInterval ? formatUpdateInterval(deviceResponse.data.updateInterval) : "-";
              } catch (error) {
                console.error(
                  `Failed to fetch additional tracker information for device ${deviceIdWithoutPrefix}:`,
                  error
                );
              }

              return {
                deviceId: deviceIdWithoutPrefix,
                name: newDevice.name || "Unknown",
                batteryStatus: batteryStatus,
                temperature: temperature,
                updateInterval: updateInterval,
              };
            })
          );

          setTrackerInformation(updatedTrackerInformation);

          // Call fetchMarkers here after fetchTrackerInformation is completed
          fetchMarkers();
        }
      } else {
        throw new Error(
          `Failed to fetch tracker information: ${response.status}, ${response.statusText}`
        );
      }
    } catch (error) {
      alert(
        "Failed to fetch tracker information. Either your API key or your application key is wrong"
      );
    }
  };

  // Helper function to format the update interval
  const formatUpdateInterval = (interval) => {
    const minutes = interval % 60;
    const hours = Math.floor(interval / 60);
    if (hours > 0) {
      return `${hours}h ${minutes}m`;
    } else {
      return `${minutes}m`;
    }
  };

  // Close menu when clicking outside
  useEffect(() => {
    const handleClickOutside = (event) => {
      if (
        settingsMenuRef.current &&
        !settingsMenuRef.current.contains(event.target)
      ) {
        setSettingsMenuOpen(false);
      }
      if (
        addDeviceMenuRef.current &&
        !addDeviceMenuRef.current.contains(event.target)
      ) {
        setAddDeviceMenuOpen(false);
      }
      if (
        mapTileMenuRef.current &&
        !mapTileMenuRef.current.contains(event.target)
      ) {
        setMapTileMenuOpen(false);
      }
    };

    document.addEventListener("mousedown", handleClickOutside);
    return () => {
      document.removeEventListener("mousedown", handleClickOutside);
    };
  }, [isSettingsMenuOpen, isAddDeviceMenuOpen, isMapTileMenuOpen]);

  return (
    <div className="map-container">
      <button className="menu-button" onClick={handleOpenSettingsMenu}>
        <ListIcon />
      </button>
      <button className="add-button" onClick={handleOpenAddDeviceMenu}>
        <AddIcon />
      </button>
      <button className="map-tile-button" onClick={handleOpenMapTileMenu}>
        <LayersIcon />
      </button>
      <MapComponent
        center={center}
        zoomLevel={zoomLevel}
        drawMarkersAtZoomLevel={drawMarkersAtZoomLevel}
        allMarkers={filteredAllMarkers}
        useCustomClusterIcon={false}
        trackerInformation={trackerInformation}
        selectedMapTile={selectedMapTile}
      />
      <SettingsMenu
        isOpen={isSettingsMenuOpen}
        handleClose={handleCloseSettingsMenu}
        trackerInformation={trackerInformation}
        handleShowLocationSwitch={handleShowLocationSwitch}
        handleShowMovement={handleShowMovementSwitch}
        handleTrackerInformationUpdate={handleTrackerInformationUpdate}
        markers={markers}
        ref={settingsMenuRef}
        dataLoaded={dataLoaded}
      />
      <AddDeviceMenu
        isOpen={isAddDeviceMenuOpen}
        handleClose={handleCloseAddDeviceMenu}
        onDeviceAdded={handleAddDeviceSuccess}
        ref={addDeviceMenuRef}
      />
      <MapTileMenu
        isOpen={isMapTileMenuOpen}
        handleClose={handleCloseMapTileMenu}
        handleTileChange={handleTileChange}
        ref={mapTileMenuRef}
      />
    </div>
  );
}