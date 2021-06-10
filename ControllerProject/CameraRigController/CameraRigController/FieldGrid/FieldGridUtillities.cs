using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;
using System.Collections.ObjectModel;
using CameraRigController.FieldGrid.Editor.ViewModel;
using System.ComponentModel;
using System.Collections;
using System.Windows;

namespace CameraRigController.FieldGrid
{
    class FieldGridSupportedTypeInfo
    {
        public FieldGridSupportedTypeInfo(Type objectType, Type editorType, Type[] attributeTypes)
        {
            ObjectType = objectType;
            EditorType = editorType;
            AttributeTypes = attributeTypes;
        }

        public Type ObjectType { get; }
        public Type EditorType { get; }
        public Type[] AttributeTypes { get; }
    }
    public static class FieldGridUtillities
    {
        private static List<FieldGridSupportedTypeInfo> _supportedTypes { get; } = new List<FieldGridSupportedTypeInfo>();

        static FieldGridUtillities()
        {
            var types = Assembly.GetExecutingAssembly().GetTypes();
            foreach (var type in types)
            {
                var att = type.GetCustomAttribute<FieldGridEditorAttribute>();
                if (att != null)
                {
                    _supportedTypes.Add(new FieldGridSupportedTypeInfo(att.EditedObjectType, type, att.AttributeTypes));
                }
            }
        }

        public static ObservableCollection<EditorViewModelBase> ToVMCollection(object obj)
        {
            var collection = new ObservableCollection<EditorViewModelBase>();

            var properties = obj.GetType().GetProperties();
            var events = obj.GetType().GetEvents();

            MethodInfo objPropertyChanged;
            foreach (var e in events)
            {
                if (e.Name == "PropertyChanged")
                {
                    objPropertyChanged = e.GetAddMethod();
                    objPropertyChanged.Invoke(obj, new[] {(PropertyChangedEventHandler)((object sender, PropertyChangedEventArgs args) =>
                    {
                        var propertyEditor = collection.FirstOrDefault((p) => p.PropertyName == args.PropertyName);
                        if (propertyEditor == null) return;
                        var objProperty = properties.First((op) => op.Name == args.PropertyName);
                        var val = objProperty.GetValue(obj);
                        if (!propertyEditor.ObjectValue.Equals(val))
                        {
                            propertyEditor.ObjectValue = val;
                        }
                    }) });
                }
            }

            foreach (var p in properties)
            {
                if (IsSupported(p.PropertyType))
                {
                    var typeInfo = GetSupportedTypeInfo(p);
                    var editor = (EditorViewModelBase)typeInfo.EditorType.GetConstructor(new Type[] { }).Invoke(new object[] { });
                    editor.PropertyName = p.Name;
                    editor.PropertyChanged += (sender, args) =>
                    {
                        if (args.PropertyName == "ObjectValue" && !p.GetValue(obj).Equals(editor.ObjectValue)) p.SetValue(obj, editor.ObjectValue);
                    };
                    editor.ObjectValue = p.GetValue(obj);
                    editor.DisplayName = p.GetCustomAttribute<DisplayNameAttribute>()?.DisplayName ?? NicifyName(p.Name);
                    collection.Add(editor);
                }
            }



            return collection;
        }

        private static FieldGridSupportedTypeInfo GetSupportedTypeInfo(PropertyInfo memberInfo)
        {
            return _supportedTypes.Find((x) =>
            {
                var atts = memberInfo.GetCustomAttributes();
                foreach (var att in x.AttributeTypes)
                {
                    if (!atts.Any((a) => a.GetType() == att)) return false;
                }
                return true;
            });
        }

        public static bool IsSupported(Type type)
        {
            return _supportedTypes.Exists((o) => o.ObjectType == type);
        }

        public static string NicifyName(string name)
        {
            var sb = new StringBuilder();
            bool numberSequence = false;
            bool capitalSequence = false;
            for (int i = 0; i < name.Length; i++)
            {
                if (char.IsLetter(name[i]))
                {
                    if (i == 0)
                    {
                        if (char.IsUpper(name[1])) capitalSequence = true;
                        else sb.Append(char.ToUpper(name[i]));
                    }
                    else
                    {
                        if (char.IsUpper(name[i]))
                        {
                            if (char.IsUpper(name[i-1]))
                            {
                                if (!capitalSequence)
                                {
                                    capitalSequence = true;
                                    
                                }
                                sb.Append(name[i - 1]);
                            }
                            else
                            {
                                if (numberSequence && i + 1 < name.Length && char.IsUpper(name[i + 1]))
                                {
                                    sb.Append(' ');
                                }
                                else
                                {
                                    sb.Append(' ');
                                    sb.Append(char.ToLower(name[i]));
                                }
                            }
                            if (i == name.Length - 1 && capitalSequence)
                            {
                                sb.Append(name[i]);
                            }
                        }
                        else
                        {
                            if (capitalSequence)
                            {
                                capitalSequence = false;
                                sb.Append(' ');
                                sb.Append(char.ToLower(name[i - 1]));
                            }
                            sb.Append(name[i]);
                        }
                    }
                    numberSequence = false;
                }
                else if (char.IsDigit(name[i]))
                {
                    if (capitalSequence) sb.Append(name[i - 1]);
                    if (!numberSequence)
                    {
                        numberSequence = true;
                        sb.Append(' ');
                    }
                    sb.Append(name[i]);
                    capitalSequence = false;
                }
            }
            return sb.ToString();
        }

        public static IEnumerable<LocalValueEntry> ToEnumerable(this LocalValueEnumerator enumerator)
        {
            while (enumerator.MoveNext()) yield return enumerator.Current;
        }
    }
}
