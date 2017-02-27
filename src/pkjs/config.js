module.exports = [
  {
    "type": "heading",
    "defaultValue": "GC Watch Configuration"
  },
  {
    "type": "text",
    "defaultValue": "Specify the colors."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },
      {
        "type": "color",
        "messageKey": "backgroundColor",
        "defaultValue": "0x000000",
        "label": "Background Color BT ok"
      },
      {
        "type": "color",
        "messageKey": "backgroundBTColor",
        "defaultValue": "0xFFFFFF",
        "label": "BT Error Background Color"
      },
      {
        "type": "color",
        "messageKey": "textColor",
        "defaultValue": "0xFFFFFF",
        "label": "Text Color"
      },
      {
        "type": "color",
        "messageKey": "batteryColor",
        "defaultValue": "0xFFFFFF",
        "label": "Battery Color"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];